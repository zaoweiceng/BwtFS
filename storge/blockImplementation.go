package storge

import (
	"bytes"
	"crypto/sha256"
	"log"
	"os"
	"time"
	"unsafe"

	"math/rand"

	"example.com/fs/V1/base"
	"github.com/samborkent/uuidv7"
)

func NewFile(filename string, filesize int64, owner [32]byte) (*File, error) {
	/*
		1. Create a file with the given size
		2. Create a file metadata
		3. Create a file info
		4. Create a bitmap
		5. Write metadata to the file
		6. Write file info to the file
		7. Write bitmap to the file
		Args:
			filename: string - the name of the file
			filesize: int64 - the blocks of the file in number
			owner: [32]byte - the owner of the file
		Returns:
			*File - the file object
	*/
	fileid := __getUUID()
	bitmap := uint64(time.Now().Unix())%uint64(filesize) - 2
	if bitmap <= 0 {
		bitmap = 1
	}
	bitmap_num := uint64((filesize + 8*base.BLOCK_SIZE - 1) / (8 * base.BLOCK_SIZE))
	err := __createFile(filename, filesize*base.BLOCK_SIZE)
	if err != nil {
		return nil, err
	}
	create_time := time.Now().Unix()
	fm := FileMetadata{base.VERSION, fileid, owner, create_time, 1, uint64(filesize - 1), base.BLOCK_SIZE, uint64(filesize * base.BLOCK_SIZE / base.UNIT / 1024), bitmap}
	fi := FileInfo{fileid, owner, create_time, create_time, create_time, 1, uint64(filesize) - 1, uint64(filesize * base.BLOCK_SIZE), base.BLOCK_SIZE, bitmap + bitmap_num, uint64(filesize), __checkSum(fm.toBinary())}
	f := File{filename, fm, fi, nil}
	bitmap_data := Bitmap{make([]byte, filesize)}
	bitmap_data.initBitmap()
	bitmap_data.SetBit(0)
	bitmap_data.SetBit(bitmap)
	for i := uint64(1); i < bitmap_num; i++ {
		bitmap_data.SetBit(bitmap + i)
	}
	bitmap_data.SetBit(uint64(filesize - 1))
	// fmt.Println("bitmap", bitmap_data.data[0:100])
	f.openFile(filename)
	f.WriteBlockBitmap(bitmap_data)
	f.writeFileInfo()
	f.writeMetadata()
	return &f, nil
}

func OpenFile(filename string) (*File, error) {
	f := File{filename, FileMetadata{}, FileInfo{}, nil}
	err := f.openFile(filename)
	if err != nil {
		return nil, err
	}
	f.fm.fromBinary(f.readMetadata())
	f.fi.fromBinary(f.readFileInfo())
	f.fi.visited = time.Now().Unix()
	f.writeFileInfo()
	return &f, nil
}

func (f *FileMetadata) toBinary() []byte {
	var buf bytes.Buffer
	buf.Write([]byte{f.version})
	buf.Write(f.fileid[:])
	buf.Write(f.owner[:])
	buf.Write((*[8]byte)(unsafe.Pointer(&f.created))[:])
	buf.Write((*[8]byte)(unsafe.Pointer(&f.start_block))[:])
	buf.Write((*[8]byte)(unsafe.Pointer(&f.end_block))[:])
	buf.Write((*[8]byte)(unsafe.Pointer(&f.block_size))[:])
	buf.Write((*[8]byte)(unsafe.Pointer(&f.file_size))[:])
	buf.Write((*[8]byte)(unsafe.Pointer(&f.bitmap))[:])
	return buf.Bytes()
}

func (f *FileMetadata) fromBinary(data []byte) {
	f.version = data[0]
	copy(f.fileid[:], data[1:17])
	copy(f.owner[:], data[17:49])
	f.created = *(*int64)(unsafe.Pointer(&data[49]))
	f.start_block = *(*uint64)(unsafe.Pointer(&data[57]))
	f.end_block = *(*uint64)(unsafe.Pointer(&data[65]))
	f.block_size = *(*uint64)(unsafe.Pointer(&data[73]))
	f.file_size = *(*uint64)(unsafe.Pointer(&data[81]))
	f.bitmap = *(*uint64)(unsafe.Pointer(&data[89]))
}

func (f *FileInfo) toBinary() []byte {
	var buf bytes.Buffer
	buf.Write(f.fileid[:])
	buf.Write(f.owner[:])
	buf.Write((*[8]byte)(unsafe.Pointer(&f.created))[:])
	buf.Write((*[8]byte)(unsafe.Pointer(&f.changed))[:])
	buf.Write((*[8]byte)(unsafe.Pointer(&f.visited))[:])
	buf.Write((*[8]byte)(unsafe.Pointer(&f.start_block))[:])
	buf.Write((*[8]byte)(unsafe.Pointer(&f.end_block))[:])
	buf.Write((*[8]byte)(unsafe.Pointer(&f.file_size))[:])
	buf.Write((*[8]byte)(unsafe.Pointer(&f.block_size))[:])
	buf.Write((*[8]byte)(unsafe.Pointer(&f.bitmap_end))[:])
	buf.Write((*[8]byte)(unsafe.Pointer(&f.bitmap_size))[:])
	buf.Write(f.checksum[:])
	return buf.Bytes()
}

func (f *FileInfo) fromBinary(data []byte) {
	copy(f.fileid[:], data[0:16])
	copy(f.owner[:], data[16:48])
	f.created = *(*int64)(unsafe.Pointer(&data[48]))
	f.changed = *(*int64)(unsafe.Pointer(&data[56]))
	f.visited = *(*int64)(unsafe.Pointer(&data[64]))
	f.start_block = *(*uint64)(unsafe.Pointer(&data[72]))
	f.end_block = *(*uint64)(unsafe.Pointer(&data[80]))
	f.file_size = *(*uint64)(unsafe.Pointer(&data[88]))
	f.block_size = *(*uint64)(unsafe.Pointer(&data[96]))
	f.bitmap_end = *(*uint64)(unsafe.Pointer(&data[104]))
	f.bitmap_size = *(*uint64)(unsafe.Pointer(&data[112]))
	copy(f.checksum[:], data[120:152])
}

func __createFile(filename string, filesize int64) error {
	file, err := os.OpenFile(filename, os.O_RDWR|os.O_CREATE|os.O_TRUNC, 0666)
	if err != nil {
		log.Printf("Failed to open file: %s", err)
		return err
	}
	defer file.Close()
	err = file.Truncate(filesize)
	if err != nil {
		log.Printf("Failed to truncate file: %s", err)
		return err
	}
	r := rand.New(rand.NewSource(time.Now().UnixNano()))
	bufferSize := 1024 * 1024
	buffer := make([]byte, bufferSize)
	for written := int64(0); written < filesize; {
		for i := range buffer {
			buffer[i] = byte(r.Intn(256))
		}
		n, err := file.Write(buffer)
		if err != nil {
			log.Printf("Failed to write to file: %s", err)
			return err
		}
		written += int64(n)
		if n < len(buffer) {
			break
		}
	}
	return nil
}

func __checkSum(b []byte) [32]byte {
	hash := sha256.New()
	hash.Write(b)
	return [32]byte(hash.Sum(nil))
}

func (f *File) openFile(filename string) error {
	file, err := os.OpenFile(filename, os.O_RDWR, 0666)
	if err != nil {
		log.Print(err)
		return err
	}
	f.f = file
	return nil
}

func (f *File) Close() {
	f.f.Close()
}

func (f *File) readMetadata() []byte {
	var data [unsafe.Sizeof(FileMetadata{})]byte
	f.f.ReadAt(data[:], 0)
	return data[:]
}

func (f *File) writeMetadata() {
	f.f.WriteAt(f.fm.toBinary(), 0)
}

func (f *File) readFileInfo() []byte {
	var data [unsafe.Sizeof(FileInfo{})]byte
	f.f.ReadAt(data[:], int64(f.fm.end_block)*int64(f.fm.block_size))
	return data[:]
}

func (f *File) writeFileInfo() {
	f.f.WriteAt(f.fi.toBinary(), int64(f.fm.end_block)*int64(f.fm.block_size))
}

func __getUUID() [16]byte {
	uuid := uuidv7.New()
	var fileid [16]byte
	copy(fileid[:], uuid[:])
	return fileid
}

func (f *File) ReadBlock(block_num uint64) []byte {
	if block_num > f.fi.end_block+1 || block_num < f.fi.start_block-1 {
		return make([]byte, 0)
	}
	data := make([]byte, f.fm.block_size)
	f.f.ReadAt(data[:], int64(block_num*f.fm.block_size))
	return data[:]
}

func (f *File) WriteBlock(block_num uint64, data []byte) uint64 {
	if block_num > f.fi.end_block+1 || block_num < f.fi.start_block {
		return 0
	}
	f.fi.changed = time.Now().Unix()
	f.writeFileInfo()
	f.f.WriteAt(data, int64(block_num*f.fm.block_size))
	return block_num
}

func (f *File) GetFiBitmapSize() uint64 {
	return f.fi.bitmap_size
}

func (f *File) ReadBlockBitmap() Bitmap {
	data := make([]byte, f.fi.bitmap_size)
	f.f.ReadAt(data[:], int64(f.fm.bitmap)*int64(f.fm.block_size))
	return Bitmap{data[:]}
}

func (f *File) WriteBlockBitmap(bitmap Bitmap) {
	data := make([]byte, f.fi.bitmap_size)
	copy(data[:], bitmap.data)
	f.f.WriteAt(data, int64(f.fm.bitmap)*int64(f.fm.block_size))
}

func (b *Bitmap) SetBit(position uint64) {
	b.data[position/8] |= 1 << (position % 8)
}

func (b *Bitmap) ClearBit(position uint64) {
	b.data[position/8] &= ^(1 << (position % 8))
}

func (b *Bitmap) GetBit(position uint64) bool {
	return b.data[position/8]&(1<<(position%8)) == 0
}

func (b *Bitmap) initBitmap() {
	for i := range b.data {
		b.data[i] = 0
	}
}
