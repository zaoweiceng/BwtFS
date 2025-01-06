package tool

import (
	"errors"

	"example.com/fs/V1/base"
	"example.com/fs/V1/bwt"
)

var (
	files []*File
)

func init() {
	INFO("init bwt file system")
}

// ---------------------------------------------------------
// ---------------------------------------------------------
// ---------------------------------------------------------
// ---------------------BWT System--------------------------
func CreateFileSystem(system_name string, filesize uint64, owner [32]byte) {
	INFO("CreateFileSystem")
	CreateFile(system_name, filesize, owner)
}

func GetFreeSize(system_name string) uint64 {
	file_Meta_data, err := ReadFile(system_name, 0)
	if err != nil {
		ERROR(err, "GetFreeSize: ReadFile")
		return 0
	}
	file_meta := bwt.FileMetadata{}
	file_meta.FromBinary(file_Meta_data)
	file_info_data, err := ReadFile(system_name, file_meta.End_block)
	if err != nil {
		ERROR(err, "GetFreeSize: ReadFile")
		return 0
	}
	file_info := bwt.FileInfo{}
	file_info.FromBinary(file_info_data)
	bitmaps := []byte{}
	for i := file_meta.Bitmap; i <= file_info.Bitmap_end; i++ {
		bitmap, _ := ReadFile(system_name, i)
		bitmaps = append(bitmaps, bitmap...)
		if len(bitmaps)*8 >= int(file_info.Bitmap_size) {
			break
		}
	}
	free_size := uint64(0)
	bit_len := uint64(0)
	for _, b := range bitmaps {
		for i := 0; i < 8; i++ {
			if b&(1<<uint(i)) == 0 {
				free_size++
			}
			bit_len++
			if bit_len >= file_info.Bitmap_size {
				break
			}
		}
		if bit_len >= file_info.Bitmap_size {
			break
		}
	}
	return free_size * file_info.Block_size / base.MB
}

func GetSize(system_name string) uint64 {
	file_Meta_data, err := ReadFile(system_name, 0)
	if err != nil {
		ERROR(err, "GetSize: ReadFile")
		return 0
	}
	file_meta := bwt.FileMetadata{}
	file_meta.FromBinary(file_Meta_data)
	return file_meta.File_size
}

func GetOwner(system_name string) [32]byte {
	file_Meta_data, err := ReadFile(system_name, 0)
	if err != nil {
		ERROR(err, "GetOwner: ReadFile")
		return [32]byte{}
	}
	file_meta := bwt.FileMetadata{}
	file_meta.FromBinary(file_Meta_data)
	return file_meta.Owner
}

func GetCreated(system_name string) int64 {
	file_Meta_data, err := ReadFile(system_name, 0)
	if err != nil {
		ERROR(err, "GetCreated: ReadFile")
		return 0
	}
	file_meta := bwt.FileMetadata{}
	file_meta.FromBinary(file_Meta_data)
	return file_meta.Created
}

func GetChanged(system_name string) int64 {
	file_Meta_data, err := ReadFile(system_name, 0)
	if err != nil {
		ERROR(err, "GetChanged: ReadFile")
		return 0
	}
	file_meta := bwt.FileMetadata{}
	file_meta.FromBinary(file_Meta_data)
	file_info_data, err1 := ReadFile(system_name, file_meta.End_block)
	if err1 != nil {
		ERROR(err1, "GetChanged: ReadFile")
		return 0
	}
	file_info := bwt.FileInfo{}
	file_info.FromBinary(file_info_data)
	return file_info.Changed
}

func GetVisited(system_name string) int64 {
	file_Meta_data, err := ReadFile(system_name, 0)
	if err != nil {
		ERROR(err, "GetVisited: ReadFile")
		return 0
	}
	file_meta := bwt.FileMetadata{}
	file_meta.FromBinary(file_Meta_data)
	file_info_data, err1 := ReadFile(system_name, file_meta.End_block)
	if err1 != nil {
		ERROR(err1, "GetVisited: ReadFile")
		return 0
	}
	file_info := bwt.FileInfo{}
	file_info.FromBinary(file_info_data)
	return file_info.Visited
}

func PutFile(system_name string, filename string, file_description string, owner [32]byte) int {
	INFO("Put a New File to BWT System")
	f := NewFile(system_name, filename, file_description, owner)
	files = append(files, f)
	return len(files) - 1
}

func GetFileName(system_name string, token string) string {
	token_data, _ := DecodeToken(token)
	start, _ := BytesToData(token_data)
	file_Meta_data, err := ReadFile(system_name, start)
	if err != nil {
		ERROR(err, "GetFileName: ReadFile")
		return ""
	}
	file_meta := bwt.MetaNode{}
	file_meta.FromBinary(file_Meta_data)
	return file_meta.GetFilename()
}

func GetFileDescription(system_name string, token string) string {
	token_data, _ := DecodeToken(token)
	start, _ := BytesToData(token_data)
	file_Meta_data, err := ReadFile(system_name, start)
	if err != nil {
		ERROR(err, "GetFileDescription: ReadFile")
		return ""
	}
	file_meta := bwt.MetaNode{}
	file_meta.FromBinary(file_Meta_data)
	return file_meta.GetFileDescription()
}

func GetFileOwner(system_name string, token string) [32]byte {
	token_data, _ := DecodeToken(token)
	start, _ := BytesToData(token_data)
	file_Meta_data, err := ReadFile(system_name, start)
	if err != nil {
		ERROR(err, "GetFileOwner: ReadFile")
		return [32]byte{}
	}
	file_meta := bwt.MetaNode{}
	file_meta.FromBinary(file_Meta_data)
	return file_meta.GetOwner()
}

func WriteData(file_id int, data []byte) {
	files[file_id].AddData(data)
}

func FinishWriteData(file_id int) {
	files[file_id].FinishWrite()
}

func DownLoadFile(file_id int, target string) {
	files[file_id].DownLoadFile(target)
}

func ReadBytes(file_id int) ([]byte, error) {
	return files[file_id].ReadBytes()
}

func ReadAt(file_id int, data []byte, offset int64) error {
	return files[file_id].ReadAt(uint64(offset), data)
}

func DeleteFile(system_name string, token string) error {
	file, err := OpenFile__(token, system_name)
	if err != nil {
		err = errors.New("open file error")
		ERROR(err, "DeleteFile: OpenFile__")
		return err
	}
	file.LoadWholeFile()
	for i := 0; i < len(file.node_list); i++ {
		if file.node_list[i].isBlack || file.node_list[i].isWhite || file.node_list[i].isIndex {
			ClearBitmap(system_name, file.node_list[i].bitmap)
		}
	}
	return nil
}

func OpenFile(system_name string, token string) (int, error) {
	file, err := OpenFile__(token, system_name)
	if err != nil {
		err = errors.New("open file error")
		ERROR(err, "OpenFile: OpenFile__")
		return -1, err
	}
	files = append(files, file)
	return len(files) - 1, nil
}
