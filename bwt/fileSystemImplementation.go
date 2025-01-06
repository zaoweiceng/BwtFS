package bwt

import (
	"bytes"
	"errors"
	"unsafe"

	"math/rand"
)

// ------------------------------------ MetaNode ------------------------------------

func (m *MetaNode) InitMetaNode(filename string, file_description string, owner [32]byte, bitmap []uint64) *MetaNode {
	m.Filaname = []byte(filename)
	m.filename_size = len(m.Filaname)
	if m.filename_size > 255 {
		m.filename_size = 255
		m.Filaname = m.Filaname[:255]
	}
	m.file_description_size = len(file_description)
	m.FileDescription = []byte(file_description)
	if m.file_description_size > 1024 {
		m.file_description_size = 1024
		m.FileDescription = []byte(file_description[:1024])
	}
	m.Owner = owner
	m.bitmap = bitmap
	return m
}

func (m *MetaNode) GetFilename() string {
	return string(m.Filaname)
}

func (m *MetaNode) GetFileDescription() string {
	return string(m.FileDescription)
}

func (m *MetaNode) GetOwner() [32]byte {
	return m.Owner
}

func (m *MetaNode) GetBitmap() []uint64 {
	return m.bitmap
}

func (m *MetaNode) SetBitmap(bitmap []uint64) {
	m.bitmap = bitmap
}

func (m *MetaNode) ToBinary() []byte {
	var buf bytes.Buffer
	buf.Write((*[8]byte)(unsafe.Pointer(&m.filename_size))[:])
	buf.Write(m.Filaname)
	buf.Write((*[8]byte)(unsafe.Pointer(&m.file_description_size))[:])
	buf.Write(m.FileDescription)
	buf.Write(m.Owner[:])
	for _, b := range m.bitmap {
		buf.Write((*[8]byte)(unsafe.Pointer(&b))[:])
	}
	return buf.Bytes()
}

func (m *MetaNode) FromBinary(b []byte) {
	m.filename_size = *(*int)(unsafe.Pointer(&b[0]))
	m.Filaname = b[8 : 8+m.filename_size]
	m.file_description_size = *(*int)(unsafe.Pointer(&b[8+m.filename_size]))
	m.FileDescription = b[16+m.filename_size : 16+m.filename_size+m.file_description_size]
	copy(m.Owner[:], b[16+m.filename_size+m.file_description_size:16+m.filename_size+m.file_description_size+32])
	for i := 16 + m.filename_size + m.file_description_size + 32; i < len(b); i += 8 {
		m.bitmap = append(m.bitmap, *(*uint64)(unsafe.Pointer(&b[i])))
	}
}

// ------------------------------------ END MetaNode ------------------------------------

// ------------------------------------ WhiteNode ---------------------------------------

func (w *WhiteNode) GetDataSize() int {
	return w.data_size
}

func (w *WhiteNode) GetBlockIndex() int {
	return w.block_index
}

func (w *WhiteNode) GetData() []byte {
	return w.data[:w.data_size]
}

func (w *WhiteNode) ToBinary() []byte {
	var buf bytes.Buffer
	buf.Write((*[8]byte)(unsafe.Pointer(&w.block_index))[:])
	buf.Write((*[8]byte)(unsafe.Pointer(&w.data_size))[:])
	buf.Write(w.data[:])
	return buf.Bytes()
}

func (w *WhiteNode) FromBinary(data []byte) {
	w.block_index = *(*int)(unsafe.Pointer(&data[0]))
	w.data_size = *(*int)(unsafe.Pointer(&data[8]))
	copy(w.data[:], data[16:])
}

func (w *WhiteNode) InitWhiteNode(block_index int, data_size int, data []byte) *WhiteNode {
	w.data_size = data_size
	w.block_index = block_index
	copy(w.data[:], data[:])
	for i := data_size; i < len(w.data); i++ {
		w.data[i] = byte(rand.Intn(256))
	}
	return w
}

// ------------------------------------ END WhiteNode ------------------------------------

// ------------------------------------ BlackNode ----------------------------------------

func (b *BlackNode) GetDataSize() int {
	return b.data_size
}

func (b *BlackNode) GetBlockIndex() int {
	return b.block_index
}

func (b *BlackNode) GetData() []byte {
	return b.data[:b.data_size]
}

func (b *BlackNode) InitBlackNode(block_index int, data_size int, data []byte) *BlackNode {
	b.data_size = data_size
	b.block_index = block_index
	copy(b.data[:], data[:])
	for i := data_size; i < len(b.data); i++ {
		b.data[i] = byte(rand.Intn(256))
	}
	return b
}

func (b *BlackNode) ToBinary() []byte {
	var buf bytes.Buffer
	buf.Write(b.data[:])
	buf.Write((*[8]byte)(unsafe.Pointer(&b.block_index))[:])
	buf.Write((*[8]byte)(unsafe.Pointer(&b.data_size))[:])
	return buf.Bytes()
}

func (b *BlackNode) FromBinary(data []byte) {
	copy(b.data[:], data[:len(data)-2*8])
	b.block_index = *(*int)(unsafe.Pointer(&data[len(data)-2*8]))
	b.data_size = *(*int)(unsafe.Pointer(&data[len(data)-8]))
}

// ------------------------------------ END BlackNode ------------------------------------

// ------------------------------------ IndexNode ----------------------------------------

func (i *IndexNode) InitIndexNode(node_size int, entry []IndexEntry) *IndexNode {
	i.node_size = int32(node_size)
	copy(i.entry[:], entry)
	return i
}

func (i *IndexNode) AddEntry(entry IndexEntry) error {
	if int(i.node_size) >= len(i.entry) {
		return errors.New("IndexNode is full")
	}
	i.entry[i.node_size] = entry
	i.node_size++
	return nil
}

func (i *IndexNode) GetEntry(index int) (IndexEntry, error) {
	if index >= int(i.node_size) || i.node_size == 0 {
		return IndexEntry{}, errors.New("index out of range")
	}
	return i.entry[index], nil
}

func (i *IndexNode) SetEntry(index int, entry IndexEntry) error {
	if index >= int(i.node_size) || i.node_size == 0 {
		return errors.New("index out of range")
	}
	i.entry[index] = entry
	return nil
}

func (i *IndexNode) MaxSize() int {
	return len(i.entry)
}

func (i *IndexNode) ToBinary() []byte {
	var buf bytes.Buffer
	buf.Write((*[8]byte)(unsafe.Pointer(&i.node_size))[:])
	for _, e := range i.entry[:] {
		buf.Write(e.ToBinary())
	}
	return buf.Bytes()
}

func (i *IndexNode) FromBinary(b []byte) {
	i.node_size = *(*int32)(unsafe.Pointer(&b[0]))
	for j, l := 8, 0; l < int(i.node_size); j += 16 {
		i.entry[l].FromBinary(b[j : j+16])
		l++
	}
}

// ------------------------------------ END IndexNode ------------------------------------

// ------------------------------------ IndexEntry ---------------------------------------
func (i *IndexEntry) InitIndexEntry(black bool, white bool, valid bool, index bool, bitmap uint64) *IndexEntry {
	i.black = black
	i.white = white
	i.valid = valid
	i.index = index
	i.bitmap = bitmap
	return i
}

func GetRandomByteBasedOnBool(flag bool) byte {
	var num byte
	for {
		num = byte(rand.Intn(256))
		if flag && num%2 == 0 {
			return num
		} else if !flag && num%2 != 0 {
			return num
		}
	}
}

func (i *IndexEntry) ToBinary() []byte {
	var buf bytes.Buffer
	b := make([]byte, 8)
	b[0] = GetRandomByteBasedOnBool(i.black)
	b[1] = GetRandomByteBasedOnBool(i.white)
	b[2] = GetRandomByteBasedOnBool(i.valid)
	b[3] = GetRandomByteBasedOnBool(i.index)
	for j := 4; j < 8; j++ {
		b[j] = byte(rand.Intn(256))
	}
	buf.Write(b)
	buf.Write((*[8]byte)(unsafe.Pointer(&i.bitmap))[:])
	return buf.Bytes()
}

func (i *IndexEntry) FromBinary(b []byte) {
	i.black = b[0]%2 == 0
	i.white = b[1]%2 == 0
	i.valid = b[2]%2 == 0
	i.index = b[3]%2 == 0
	i.bitmap = *(*uint64)(unsafe.Pointer(&b[8]))
}

func (i *IndexEntry) IsValid() bool {
	return (i.black != i.white) && i.valid
}

func (i *IndexEntry) GetBitmap() uint64 {
	return i.bitmap
}

func (i *IndexEntry) IsBlack() bool {
	return i.black
}

func (i *IndexEntry) IsWhite() bool {
	return i.white
}

func (i *IndexEntry) IsIndex() bool {
	return i.index
}

func (i *IndexEntry) SetIndex(index bool) {
	i.index = index
}

// ------------------------------------ END IndexEntry ------------------------------------
