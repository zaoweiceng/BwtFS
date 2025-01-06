package storge

import "os"

type FileMetadata struct {
	version     uint8    //文件系统的版本号，不同版本的文件系统可能有不同的文件格式
	fileid      [16]byte //文件的唯一标识符
	owner       [32]byte //文件的所有者哈希
	created     int64    //文件的创建时间
	start_block uint64   //文件的起始块
	end_block   uint64   //文件的结束块
	block_size  uint64   //文件的块大小，单位为KB
	file_size   uint64   //文件的大小，单位为MB
	bitmap      uint64   //文件的位图的起始块
}

type FileInfo struct {
	fileid      [16]byte //文件的唯一标识符，同元信息中的fileid一致
	owner       [32]byte //文件的所有者哈希， 同元信息中的owner一致
	created     int64    //文件的创建时间，同元信息中的created一致
	changed     int64    //文件的最后修改时间
	visited     int64    //文件的最后访问时间
	start_block uint64   //文件的起始块，同元信息中的start_block一致
	end_block   uint64   //文件的结束块，同元信息中的end_block一致
	file_size   uint64   //文件的大小，单位为MB，同元信息中的file_size一致
	block_size  uint64   //文件的块大小，单位为KB，同元信息中的block_size一致
	bitmap_end  uint64   //文件的位图的结束块
	bitmap_size uint64   //文件的位图的大小，单位为位
	checksum    [32]byte //元信息的校验和
}

type File struct {
	filename string
	fm       FileMetadata
	fi       FileInfo
	f        *os.File
}

type Bitmap struct {
	data []byte
}
