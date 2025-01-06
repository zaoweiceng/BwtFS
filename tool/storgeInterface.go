package tool

import (
	"math/rand"

	"example.com/fs/V1/storge"
)

var (
	file     *storge.File
	filename *string
)

func init() {
	INFO("init stroge system")
	filename = new(string)
}

// ---------------------------------------------------------
// ---------------------------------------------------------
// ---------------------Block Manage------------------------

func CreateFile(file_name string, filesize uint64, owner [32]byte) {
	f, err := storge.NewFile(file_name, int64(filesize), owner)
	if err != nil {
		ERROR(err, "CreateFile: NewFile")
		panic(err)
	}
	file = f
	filename = &file_name
}

func ReadFile(file_name string, block_num uint64) ([]byte, error) {
	if file_name != *filename {
		var err error
		file, err = storge.OpenFile(file_name)
		if err != nil {
			ERROR(err, "ReadFile: OpenFile")
			return nil, err
		}
		filename = &file_name
	}
	return file.ReadBlock(block_num), nil
}

func WriteFile(file_name string, block_num uint64, data []byte) error {
	if file_name != *filename {
		var err error
		file, err = storge.OpenFile(file_name)
		if err != nil {
			ERROR(err, "WriteFile: OpenFile")
			return err
		}
		filename = &file_name
	}
	file.WriteBlock(block_num, data)
	return nil
}

func GetBitmap(file_name string) uint64 {
	if file_name != *filename {
		var err error
		file, err = storge.OpenFile(file_name)
		if err != nil {
			ERROR(err, "GetBitmap: OpenFile")
			return 0
		}
		filename = &file_name
	}
	bitmap := file.ReadBlockBitmap()
	num := rand.Uint64() % (file.GetFiBitmapSize())
	find := num
	for {
		if bitmap.GetBit(find) {
			return find
		}
		find++
		if find >= file.GetFiBitmapSize() {
			find = 0
		}
		if find == num {
			break
		}
	}
	return 0
}

func SetBitmap(file_name string, block_num uint64) error {
	if file_name != *filename {
		var err error
		file, err = storge.OpenFile(file_name)
		if err != nil {
			ERROR(err, "SetBitmap: OpenFile")
			return err
		}
		filename = &file_name
	}
	bitmap := file.ReadBlockBitmap()
	bitmap.SetBit(block_num)
	file.WriteBlockBitmap(bitmap)
	return nil
}

func ClearBitmap(file_name string, block_num uint64) error {
	if file_name != *filename {
		var err error
		file, err = storge.OpenFile(file_name)
		if err != nil {
			ERROR(err, "ClearBitmap: OpenFile")
			return err
		}
		filename = &file_name
	}
	bitmap := file.ReadBlockBitmap()
	bitmap.ClearBit(block_num)
	file.WriteBlockBitmap(bitmap)
	return nil
}
