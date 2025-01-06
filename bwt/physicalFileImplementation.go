package bwt

import (
	"bytes"
	"unsafe"
)

func (f *FileMetadata) FromBinary(data []byte) {
	f.Version = data[0]
	copy(f.Fileid[:], data[1:17])
	copy(f.Owner[:], data[17:49])
	f.Created = *(*int64)(unsafe.Pointer(&data[49]))
	f.Start_block = *(*uint64)(unsafe.Pointer(&data[57]))
	f.End_block = *(*uint64)(unsafe.Pointer(&data[65]))
	f.Block_size = *(*uint64)(unsafe.Pointer(&data[73]))
	f.File_size = *(*uint64)(unsafe.Pointer(&data[81]))
	f.Bitmap = *(*uint64)(unsafe.Pointer(&data[89]))
}

func (f *FileMetadata) ToBinary() []byte {
	var buf bytes.Buffer
	buf.Write([]byte{f.Version})
	buf.Write(f.Fileid[:])
	buf.Write(f.Owner[:])
	buf.Write((*[8]byte)(unsafe.Pointer(&f.Created))[:])
	buf.Write((*[8]byte)(unsafe.Pointer(&f.Start_block))[:])
	buf.Write((*[8]byte)(unsafe.Pointer(&f.End_block))[:])
	buf.Write((*[8]byte)(unsafe.Pointer(&f.Block_size))[:])
	buf.Write((*[8]byte)(unsafe.Pointer(&f.File_size))[:])
	buf.Write((*[8]byte)(unsafe.Pointer(&f.Bitmap))[:])
	return buf.Bytes()
}

func (f *FileInfo) FromBinary(data []byte) {
	copy(f.Fileid[:], data[0:16])
	copy(f.Owner[:], data[16:48])
	f.Created = *(*int64)(unsafe.Pointer(&data[48]))
	f.Changed = *(*int64)(unsafe.Pointer(&data[56]))
	f.Visited = *(*int64)(unsafe.Pointer(&data[64]))
	f.Start_block = *(*uint64)(unsafe.Pointer(&data[72]))
	f.End_block = *(*uint64)(unsafe.Pointer(&data[80]))
	f.File_size = *(*uint64)(unsafe.Pointer(&data[88]))
	f.Block_size = *(*uint64)(unsafe.Pointer(&data[96]))
	f.Bitmap_end = *(*uint64)(unsafe.Pointer(&data[104]))
	f.Bitmap_size = *(*uint64)(unsafe.Pointer(&data[112]))
	copy(f.Checksum[:], data[120:152])
}
