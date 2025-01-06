package tool

import (
	"fmt"
	"os"

	"math/rand"

	"errors"

	"example.com/fs/V1/base"
	"example.com/fs/V1/bwt"
)

type IndexNodeHeader struct {
	isBlack bool
	isWhite bool
	isIndex bool
	bitmap  uint64
	data    []byte
}

type File struct {
	System_name string
	meta_data   bwt.MetaNode
	node_list   []IndexNodeHeader
	data        []byte
}

// ------------------------------- IndexNodeHeader ----------------------------------
func newIndexNodeHeader(isBlack bool, isWhite bool, isIndex bool, bitmap uint64, data []byte) IndexNodeHeader {
	return IndexNodeHeader{
		isBlack: isBlack,
		isWhite: isWhite,
		isIndex: isIndex,
		bitmap:  bitmap,
		data:    data,
	}
}

func (inh *IndexNodeHeader) GetBitmap() uint64 {
	return inh.bitmap
}

// ------------------------------ END IndexNodeHeader -------------------------------

// ------------------------------------ File ----------------------------------------
func (f *File) InitFile(filename string, file_description string, owner [32]byte) *File {
	f.meta_data.InitMetaNode(filename, file_description, owner, []uint64{})
	return f
}

func (f *File) AddData(data []byte) {
	f.data = append(f.data, data...)
	if len(f.data) >= base.DATA_NODE_SIZE {
		f.flush()
	}
}

func (f *File) flush() {
	isBlack := rand.Intn(256)%2 == 0
	isWhite := !isBlack
	isIndex := false

	data_len := 0
	if len(f.data) > base.DATA_NODE_SIZE {
		data_len = base.DATA_NODE_SIZE
	} else {
		data_len = len(f.data)
	}

	f.node_list = append(f.node_list, newIndexNodeHeader(isBlack, isWhite, isIndex, 0, f.data[:data_len]))
	if len(f.data) > data_len {
		f.data = f.data[data_len:]
	} else {
		f.data = []byte{}
	}
	if len(f.node_list) > 250 {
		f.generateBlackWhiteTree()
	}
}

func (f *File) generateBlackWhiteTree() {
	node_num := (rand.Intn(250) + 10) % len(f.node_list)
	start_num := len(f.node_list) - node_num
	if start_num < 0 {
		start_num = 0
	}
	node_list := f.node_list[start_num : start_num+node_num]

	for i := 0; i < len(node_list); i++ {
		f.writeNode(i, node_list)
	}
	index_node := bwt.IndexNode{}
	rand.Shuffle(len(node_list), func(i, j int) {
		node_list[i], node_list[j] = node_list[j], node_list[i]
	})

	for i := 0; i < len(node_list); i++ {
		ie := bwt.IndexEntry{}
		ie.InitIndexEntry(node_list[i].isBlack, node_list[i].isWhite, true, node_list[i].isIndex, node_list[i].bitmap)
		index_node.AddEntry(ie)
	}
	if rand.Intn(256)%2 == 0 {
		inh := newIndexNodeHeader(true, false, true, 0, index_node.ToBinary())
		f.node_list = append(f.node_list[:start_num], append([]IndexNodeHeader{inh}, f.node_list[start_num+node_num:]...)...)
	} else {
		inh := newIndexNodeHeader(false, true, true, 0, index_node.ToBinary())
		f.node_list = append(f.node_list[:start_num], append([]IndexNodeHeader{inh}, f.node_list[start_num+node_num:]...)...)
	}
}

func (f *File) writeNode(index int, node_list []IndexNodeHeader) error {
	bitmap := GetBitmap(f.System_name)
	if bitmap == 0 {
		err := errors.New("file system is full")
		ERROR(err, "writeNode: GetBitmap")
		return err
	}
	SetBitmap(f.System_name, bitmap)
	if len(node_list[index].data) > base.DATA_NODE_SIZE {
		err := errors.New("data node size is too large")
		ERROR(err, "writeNode: SetBitmap")
	}
	if node_list[index].isBlack {
		black_node := bwt.BlackNode{}
		black_node.InitBlackNode(index, len(node_list[index].data), node_list[index].data)
		node_list[index].bitmap = bitmap
		WriteFile(f.System_name, bitmap, black_node.ToBinary())
	} else if node_list[index].isWhite {
		white_node := bwt.WhiteNode{}
		white_node.InitWhiteNode(index, len(node_list[index].data), node_list[index].data)
		node_list[index].bitmap = bitmap
		WriteFile(f.System_name, bitmap, white_node.ToBinary())
	}
	return nil
}

func (f *File) Flush() {
	f.flush()
}

func (f *File) FinishWrite() (string, error) {
	// 处理未完成的数据
	if len(f.data) > 0 {
		f.flush()
	}
	if len(f.node_list) == 0 {
		WARNING("No data to write")
		return "", nil
	}
	// 生成黑白树，直到节点数小于10
	for len(f.node_list) > 10 {
		f.generateBlackWhiteTree()
	}
	// 生成索引块
	// 1. 生成索引的bitmap信息
	for i := len(f.node_list); i < 16; i++ {
		insert := rand.Intn(len(f.node_list))
		f.node_list = append(f.node_list[:insert], append([]IndexNodeHeader{newIndexNodeHeader(false, false, false, 0, []byte{})}, f.node_list[insert:]...)...)
	}
	index_bitmap := []byte{}
	for i := 0; i < len(f.node_list); i++ {
		if f.node_list[i].isBlack {
			if f.node_list[i].isIndex {
				index_bitmap = append(index_bitmap, byte(2))
			} else {
				index_bitmap = append(index_bitmap, byte(0))
			}
		} else if f.node_list[i].isWhite {
			if f.node_list[i].isIndex {
				index_bitmap = append(index_bitmap, byte(3))
			} else {
				index_bitmap = append(index_bitmap, byte(1))
			}
		} else {
			index_bitmap = append(index_bitmap, byte(rand.Intn(128)+4))
		}
	}
	// 2. 写入索引节点的数据
	for i := 0; i < len(f.node_list); i++ {
		if f.node_list[i].isBlack || f.node_list[i].isWhite {
			f.writeNode(i, f.node_list)
		}
	}
	// 3. 生成索引节点
	start_bitmap := GetBitmap(f.System_name)
	if start_bitmap == 0 {
		err := errors.New("file system is full")
		ERROR(err, "FinishWrite: GetBitmap")
		return "", err
	}
	e := SetBitmap(f.System_name, start_bitmap)
	if e != nil {
		ERROR(e, "FinishWrite: SetBitmap")
		return "", e
	}
	bitmaps := []uint64{}
	for i := 0; i < len(f.node_list); i++ {
		if f.node_list[i].isBlack || f.node_list[i].isWhite {
			bitmaps = append(bitmaps, f.node_list[i].GetBitmap())
		} else {
			bitmaps = append(bitmaps, rand.Uint64())
		}
	}
	f.meta_data.SetBitmap(bitmaps)
	token := GetToken(start_bitmap, index_bitmap)
	if byte(token[0])%2 == 0 {
		index_bn := bwt.BlackNode{}
		index_bn.InitBlackNode(0, len(f.meta_data.ToBinary()), f.meta_data.ToBinary())
		WriteFile(f.System_name, start_bitmap, index_bn.ToBinary())
	} else {
		index_wn := bwt.WhiteNode{}
		index_wn.InitWhiteNode(0, len(f.meta_data.ToBinary()), f.meta_data.ToBinary())
		WriteFile(f.System_name, start_bitmap, index_wn.ToBinary())
	}
	return token, nil
}

func OpenFile__(token string, system_name string) (*File, error) {
	data, _ := DecodeToken(token)
	start_bitmap, bitmaps := BytesToData(data)
	DEBUG(fmt.Sprintf("start_bitmap: %d, bitmaps: %v", start_bitmap, bitmaps))
	meta_data := bwt.MetaNode{}
	if byte(token[0])%2 == 0 {
		index_nb := bwt.BlackNode{}
		rfd, err := ReadFile(system_name, start_bitmap)
		if err != nil {
			ERROR(err, "OpenFile__: ReadFile: open File failed")
			return nil, err
		}
		index_nb.FromBinary(rfd)
		meta_data.FromBinary(index_nb.GetData())
	} else {
		index_nw := bwt.WhiteNode{}
		rfd, err := ReadFile(system_name, start_bitmap)
		if err != nil {
			ERROR(err, "OpenFile__: ReadFile: open File failed")
			return nil, err
		}
		index_nw.FromBinary(rfd)
		meta_data.FromBinary(index_nw.GetData())
	}
	node_list := []IndexNodeHeader{}
	for i := 0; i < len(bitmaps); i++ {
		if bitmaps[i] == byte(0) {
			node_list = append(node_list, newIndexNodeHeader(true, false, false, meta_data.GetBitmap()[i], []byte{}))
		} else if bitmaps[i] == byte(1) {
			node_list = append(node_list, newIndexNodeHeader(false, true, false, meta_data.GetBitmap()[i], []byte{}))
		} else if bitmaps[i] == byte(2) {
			node_list = append(node_list, newIndexNodeHeader(true, false, true, meta_data.GetBitmap()[i], []byte{}))
		} else if bitmaps[i] == byte(3) {
			node_list = append(node_list, newIndexNodeHeader(false, true, true, meta_data.GetBitmap()[i], []byte{}))
		}
	}
	file := &File{
		System_name: system_name,
		meta_data:   meta_data,
		node_list:   node_list,
		data:        []byte{},
	}
	file.LoadWholeFile()
	return file, nil
}

func getEntryIndex(system_name string, i bwt.IndexEntry) int {
	if i.IsValid() {
		if i.IsBlack() {
			node := bwt.BlackNode{}
			rfd, err := ReadFile(system_name, i.GetBitmap())
			if err != nil {
				ERROR(err, "getEntryIndex: ReadFile: getEntryIndex failed")
				return 0
			}
			node.FromBinary(rfd)
			return node.GetBlockIndex()
		} else if i.IsWhite() {
			node := bwt.WhiteNode{}
			rfd, err := ReadFile(system_name, i.GetBitmap())
			if err != nil {
				ERROR(err, "getEntryIndex: ReadFile: getEntryIndex failed")
				return 0
			}
			node.FromBinary(rfd)
			return node.GetBlockIndex()
		}
	}
	return 0
}

func (f *File) LoadWholeFile() error {
	for i := 0; i < len(f.node_list); i++ {
		if f.node_list[i].isIndex {
			index_node := bwt.IndexNode{}
			if f.node_list[i].isBlack {
				bn := bwt.BlackNode{}
				rfd, err := ReadFile(f.System_name, f.node_list[i].bitmap)
				if err != nil {
					return err
				}
				bn.FromBinary(rfd)
				index_node.FromBinary(bn.GetData())
			} else if f.node_list[i].isWhite {
				wn := bwt.WhiteNode{}
				rfd, err := ReadFile(f.System_name, f.node_list[i].bitmap)
				if err != nil {
					return err
				}
				wn.FromBinary(rfd)
				index_node.FromBinary(wn.GetData())
			}
			sub_node_list := make([]IndexNodeHeader, index_node.MaxSize())
			for j := 0; j < index_node.MaxSize(); j++ {
				ie, _ := index_node.GetEntry(j)
				if ie.IsValid() {
					if ie.IsBlack() {
						index := getEntryIndex(f.System_name, ie)
						if index < 0 {
							ie.SetIndex(true)
							index = -index
						}
						sub_node_list[index] = newIndexNodeHeader(true, false, ie.IsIndex(), ie.GetBitmap(), []byte{})
					} else if ie.IsWhite() {
						index := getEntryIndex(f.System_name, ie)
						if index < 0 {
							ie.SetIndex(true)
							index = -index
						}
						sub_node_list[index] = newIndexNodeHeader(false, true, ie.IsIndex(), ie.GetBitmap(), []byte{})
					}
				}
			}
			f.node_list = append(f.node_list[:i], append(sub_node_list, f.node_list[i+1:]...)...)
		}
	}
	for i := 0; i < len(f.node_list); i++ {
		if !f.node_list[i].isBlack && !f.node_list[i].isWhite && !f.node_list[i].isIndex {
			f.node_list = append(f.node_list[:i], f.node_list[i+1:]...)
			i--
		}
	}
	return nil
}

func (f *File) DownLoadFile(target string) error {
	wf, err := os.OpenFile(target, os.O_RDWR|os.O_CREATE|os.O_TRUNC, 0666)
	if err != nil {
		ERROR(err, "DownLoadFile: open new file failed")
		return err
	}
	for i := 0; i < len(f.node_list); i++ {
		if f.node_list[i].isBlack {
			node := bwt.BlackNode{}
			rfd, err := ReadFile(f.System_name, f.node_list[i].bitmap)
			if err != nil {
				return err
			}
			node.FromBinary(rfd)
			wf.Write(node.GetData())
		} else if f.node_list[i].isWhite {
			node := bwt.WhiteNode{}
			rfd, err := ReadFile(f.System_name, f.node_list[i].bitmap)
			if err != nil {
				return err
			}
			node.FromBinary(rfd)
			wf.Write(node.GetData())
		}
	}
	return nil
}

func (f *File) ReadBytes() ([]byte, error) {
	data := []byte{}
	for i := 0; i < len(f.node_list); i++ {
		if f.node_list[i].isBlack {
			node := bwt.BlackNode{}
			rfd, err := ReadFile(f.System_name, f.node_list[i].bitmap)
			if err != nil {
				return nil, err
			}
			node.FromBinary(rfd)
			data = append(data, node.GetData()...)
		} else if f.node_list[i].isWhite {
			node := bwt.WhiteNode{}
			rfd, err := ReadFile(f.System_name, f.node_list[i].bitmap)
			if err != nil {
				return nil, err
			}
			node.FromBinary(rfd)
			data = append(data, node.GetData()...)
		}
	}
	return data, nil
}

func NewFile(system_name string, filename string, file_description string, owner [32]byte) *File {
	file := &File{
		System_name: system_name,
		meta_data:   bwt.MetaNode{},
		node_list:   []IndexNodeHeader{},
		data:        []byte{},
	}
	file.InitFile(filename, file_description, owner)
	return file
}

func NewFileSystem(filename string, filesize uint64, owner [32]byte) {
	CreateFile(filename, filesize, owner)
}

func DeleteFile__(token string, system_name string) {
	file, err := OpenFile__(token, system_name)
	if err != nil {
		ERROR(err, "DeleteFile__: OpenFile__: open file failed")
		return
	}
	file.LoadWholeFile()
	for i := 0; i < len(file.node_list); i++ {
		if file.node_list[i].isBlack || file.node_list[i].isWhite || file.node_list[i].isIndex {
			err := ClearBitmap(system_name, file.node_list[i].bitmap)
			if err != nil {
				WARNING(fmt.Sprintf("delete bitmap failed: %d. error: %v", file.node_list[i].bitmap, err))
			}
		}
	}
}

// func (f *File) WriteAt(offset uint64, data []byte) {
// 	data_len := uint64(len(data))
// 	start_node := int(offset / uint64(base.DATA_NODE_SIZE))
// 	end_node := int((offset + data_len) / uint64(base.DATA_NODE_SIZE))
// 	end_offset := uint64((offset + data_len) % uint64(base.DATA_NODE_SIZE))
// 	start_offset := uint64(offset % uint64(base.DATA_NODE_SIZE))
// 	delete_node_list := []uint64{}
// 	if f.node_list[start_node].isBlack {
// 		node := meta.BlackNode{}
// 		node.FromBinary(tool.ReadFile(f.system_name, f.node_list[start_node].bitmap))
// 		if data_len+start_offset < uint64(base.DATA_NODE_SIZE) {
// 			new_data := make([]byte, base.DATA_NODE_SIZE)
// 			copy(new_data, node.GetData()[:start_offset])
// 			copy(new_data[start_offset:], data)
// 			copy(new_data[start_offset+data_len:], node.GetData()[start_offset+data_len:node.GetDataSize()])
// 			node.InitBlackNode(node.GetBlockIndex(), len(new_data), new_data)
// 			tool.WriteFile(f.system_name, f.node_list[start_node].bitmap, node.ToBinary())
// 		} else {
// 			sub_data := make([]byte, start_offset)
// 			copy(sub_data, node.GetData()[0:start_offset])
// 			data = append(sub_data, data...)
// 			sub_file := File{
// 				system_name: f.system_name,
// 				meta_data:   meta.MetaNode{},
// 				node_list:   []IndexNodeHeader{},
// 				data:        data,
// 			}
// 			f.setIndex(f.node_list[start_node].bitmap)
// 			flushSubFile(&sub_file, f.node_list[start_node].bitmap, true, node.GetBlockIndex())
// 		}
// 	} else if f.node_list[start_node].isWhite {
// 		node := meta.WhiteNode{}
// 		node.FromBinary(tool.ReadFile(f.system_name, f.node_list[start_node].bitmap))
// 		if data_len+start_offset < uint64(base.DATA_NODE_SIZE) {
// 			new_data := make([]byte, base.DATA_NODE_SIZE)
// 			copy(new_data, node.GetData()[:start_offset])
// 			copy(new_data[start_offset:], data)
// 			copy(new_data[start_offset+data_len:], node.GetData()[start_offset+data_len:node.GetDataSize()])
// 			node.InitWhiteNode(node.GetBlockIndex(), len(new_data), new_data)
// 			tool.WriteFile(f.system_name, f.node_list[start_node].bitmap, node.ToBinary())
// 		} else {
// 			sub_data := make([]byte, start_offset)
// 			copy(sub_data, node.GetData()[0:start_offset])
// 			data = append(sub_data, data...)
// 			sub_file := File{
// 				system_name: f.system_name,
// 				meta_data:   meta.MetaNode{},
// 				node_list:   []IndexNodeHeader{},
// 				data:        data,
// 			}
// 			f.setIndex(f.node_list[start_node].bitmap)
// 			flushSubFile(&sub_file, f.node_list[start_node].bitmap, false, node.GetBlockIndex())
// 		}
// 	}
// 	log.Println("start_node: ", start_node, "end_node: ", end_node)
// 	if end_node != start_node {
// 		for i := start_node + 1; i < end_node; i++ {
// 			delete_node_list = append(delete_node_list, f.node_list[i].bitmap)
// 		}
// 		log.Println("delete_node_list: ", delete_node_list)
// 		f.delBitmap(delete_node_list)
// 		if f.node_list[end_node].isBlack {
// 			node := meta.BlackNode{}
// 			node.FromBinary(tool.ReadFile(f.system_name, f.node_list[end_node].bitmap))
// 			new_data := make([]byte, end_offset)
// 			copy(new_data, data[end_offset:])
// 			node.InitBlackNode(node.GetBlockIndex(), len(new_data), new_data)
// 			tool.WriteFile(f.system_name, f.node_list[end_node].bitmap, node.ToBinary())
// 		} else if f.node_list[end_node].isWhite {
// 			node := meta.WhiteNode{}
// 			node.FromBinary(tool.ReadFile(f.system_name, f.node_list[end_node].bitmap))
// 			new_data := make([]byte, end_offset)
// 			copy(new_data, data[end_offset:])
// 			node.InitWhiteNode(node.GetBlockIndex(), len(new_data), new_data)
// 			tool.WriteFile(f.system_name, f.node_list[end_node].bitmap, node.ToBinary())
// 		}
// 	}
// }

// func (f *File) delBitmap(bitmaps []uint64) {
// 	if len(bitmaps) == 0 {
// 		return
// 	}
// 	for i := 0; i < len(bitmaps); i++ {
// 		tool.ClearBitmap(f.system_name, bitmaps[i])
// 	}
// 	bi := 0
// 	for i := 0; i < len(f.node_list); i++ {
// 		if f.node_list[i].isIndex {
// 			index_node := meta.IndexNode{}
// 			if f.node_list[i].isBlack {
// 				bn := meta.BlackNode{}
// 				bn.FromBinary(tool.ReadFile(f.system_name, f.node_list[i].bitmap))
// 				index_node.FromBinary(bn.GetData())
// 			} else if f.node_list[i].isWhite {
// 				wn := meta.WhiteNode{}
// 				wn.FromBinary(tool.ReadFile(f.system_name, f.node_list[i].bitmap))
// 				index_node.FromBinary(wn.GetData())
// 			}
// 			for j := 0; j < index_node.MaxSize(); j++ {
// 				ie, _ := index_node.GetEntry(j)
// 				if ie.GetBitmap() == bitmaps[bi] {
// 					n_ie := meta.IndexEntry{}
// 					n_ie.InitIndexEntry(ie.IsBlack(), ie.IsWhite(), false, true, ie.GetBitmap())
// 					index_node.SetEntry(j, n_ie)
// 					if f.node_list[i].isBlack {
// 						bn := meta.BlackNode{}
// 						bn.FromBinary(tool.ReadFile(f.system_name, f.node_list[i].bitmap))
// 						bn.InitBlackNode(bn.GetBlockIndex(), len(index_node.ToBinary()), index_node.ToBinary())
// 						tool.WriteFile(f.system_name, f.node_list[i].bitmap, bn.ToBinary())
// 					} else if f.node_list[i].isWhite {
// 						wn := meta.WhiteNode{}
// 						wn.FromBinary(tool.ReadFile(f.system_name, f.node_list[i].bitmap))
// 						wn.InitWhiteNode(wn.GetBlockIndex(), len(index_node.ToBinary()), index_node.ToBinary())
// 						tool.WriteFile(f.system_name, f.node_list[i].bitmap, wn.ToBinary())
// 					}
// 					bi++
// 				}
// 			}
// 		}
// 	}
// }

// func flushSubFile(f *File, bit uint64, is_black bool, block_index int) {
// 	f.flush()
// 	for len(f.node_list) > 200 {
// 		f.generateBlackWhiteTree()
// 	}
// 	index_node := meta.IndexNode{}
// 	for i := 0; i < len(f.node_list); i++ {
// 		index_entry := meta.IndexEntry{}
// 		if f.node_list[i].isBlack {
// 			index_entry.InitIndexEntry(true, false, true, f.node_list[i].isIndex, f.node_list[i].bitmap)
// 		} else if f.node_list[i].isWhite {
// 			index_entry.InitIndexEntry(false, true, true, f.node_list[i].isIndex, f.node_list[i].bitmap)
// 		}
// 		index_node.AddEntry(index_entry)
// 	}
// 	if is_black {
// 		bn := meta.BlackNode{}
// 		bn.InitBlackNode(-block_index, len(index_node.ToBinary()), index_node.ToBinary())
// 		tool.WriteFile(f.system_name, bit, bn.ToBinary())
// 	} else {
// 		wn := meta.WhiteNode{}
// 		wn.InitWhiteNode(-block_index, len(index_node.ToBinary()), index_node.ToBinary())
// 		tool.WriteFile(f.system_name, bit, wn.ToBinary())
// 	}
// }

func (f *File) ReadAt(offset uint64, data []byte) error {
	data_len := uint64(len(data))
	start_node := int(offset / uint64(base.DATA_NODE_SIZE))
	start_offset := uint64(offset % uint64(base.DATA_NODE_SIZE))
	cur_offset := uint64(0)
	for cur_offset < data_len {
		if start_node >= len(f.node_list) {
			break
		}
		if f.node_list[start_node].isBlack {
			node := bwt.BlackNode{}
			rfd, err := ReadFile(f.System_name, f.node_list[start_node].bitmap)
			if err != nil {
				return err
			}
			node.FromBinary(rfd)
			copy(data[cur_offset:], node.GetData()[start_offset:])
			cur_offset += uint64(node.GetDataSize()) - start_offset
		} else if f.node_list[start_node].isWhite {
			node := bwt.WhiteNode{}
			rfd, err := ReadFile(f.System_name, f.node_list[start_node].bitmap)
			if err != nil {
				return err
			}
			node.FromBinary(rfd)
			copy(data[cur_offset:], node.GetData()[start_offset:])
			cur_offset += uint64(node.GetDataSize()) - start_offset
		}
		start_offset = 0
		start_node++
	}
	return nil
}

// ------------------------------------ END File ------------------------------------

// TODO: AddData读取到二进制数据，写入一个节点（确定下来是黑节点还是白节点），然后写入文件系统
// 然后将黑白节点的信息和获取到的bitmap信息生成IndexNodeHeader，放入File的node_list中
// 记录日志，若文件未能成功写入文件系统，将这些bitmap位置记录下来，并且清空这些位
// 若文件二进制全部写入文件系统，开始根据这些bitmap信息，黑白节点信息，构建黑白树
// 若黑白树构建成功，将黑白树的根节点的bitmap位置记录下来，写入文件系统
// 若黑白树构建还未完成构建并写入文件系统，记录下黑白树的信息，等待下一次写入
// 最终返回token给到用户

// TODO: 读取文件，根据token找到文件的索引块，根据文件的node_list，找到黑白节点，根据黑白节点找到数据
// 将node_list中的数据按流式返回给用户
