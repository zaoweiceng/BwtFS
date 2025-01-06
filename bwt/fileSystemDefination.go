package bwt

import (
	"unsafe"

	"example.com/fs/V1/base"
)

type MetaNode struct {
	filename_size         int      //文件名的长度
	Filaname              []byte   //文件名, 长度超过255的部分将被截断
	file_description_size int      //文件描述的长度
	FileDescription       []byte   //文件描述，长度超过2048的部分将被截断
	Owner                 [32]byte //文件的所有者
	bitmap                []uint64 //文件的位图
}

type WhiteNode struct {
	data_size   int                                             //数据的大小
	block_index int                                             //数据在上一层索引中的块号
	data        [base.BLOCK_SIZE - 2*unsafe.Sizeof(int(0))]byte //数据实体
}

type BlackNode struct {
	data        [base.BLOCK_SIZE - 2*unsafe.Sizeof(int(0))]byte //数据实体
	block_index int                                             //数据在上一层索引中的块号, 负数表示索引块
	data_size   int                                             //数据的大小
}

type IndexNode struct {
	node_size int32                                                                                                     //节点的大小
	entry     [(int(int(base.BLOCK_SIZE)-3*int(unsafe.Sizeof(int64(0)))) / int(unsafe.Sizeof(IndexEntry{})))]IndexEntry //位图
}

type IndexEntry struct {
	black  bool   //是否为黑节点
	white  bool   //是否为白节点
	valid  bool   //是否为有效节点
	index  bool   //是否为索引节点
	bitmap uint64 //位图位置
}
