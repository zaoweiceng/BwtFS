package bwt

type FileMetadata struct {
	Version     uint8    //文件系统的版本号，不同版本的文件系统可能有不同的文件格式
	Fileid      [16]byte //文件的唯一标识符
	Owner       [32]byte //文件的所有者哈希
	Created     int64    //文件的创建时间
	Start_block uint64   //文件的起始块
	End_block   uint64   //文件的结束块
	Block_size  uint64   //文件的块大小，单位为KB
	File_size   uint64   //文件的大小，单位为MB
	Bitmap      uint64   //文件的位图的起始块
}

type FileInfo struct {
	Fileid      [16]byte //文件的唯一标识符，同元信息中的fileid一致
	Owner       [32]byte //文件的所有者哈希， 同元信息中的owner一致
	Created     int64    //文件的创建时间，同元信息中的created一致
	Changed     int64    //文件的最后修改时间
	Visited     int64    //文件的最后访问时间
	Start_block uint64   //文件的起始块，同元信息中的start_block一致
	End_block   uint64   //文件的结束块，同元信息中的end_block一致
	File_size   uint64   //文件的大小，单位为MB，同元信息中的file_size一致
	Block_size  uint64   //文件的块大小，单位为KB，同元信息中的block_size一致
	Bitmap_end  uint64   //文件的位图的结束块
	Bitmap_size uint64   //文件的位图的大小，单位为位
	Checksum    [32]byte //元信息的校验和
}
