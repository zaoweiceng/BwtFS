package main

import (
	"fmt"
	"os"
	"time"

	"example.com/fs/V1/tool"
)

const (
	system_name = "fs.bwt"
	file_name   = "中文.pdf"
)

func uploadFile() string {
	file := tool.File{System_name: system_name}
	file.InitFile(file_name, "hello", [32]byte{})
	source := "./教程.pdf"
	source_file, _ := os.Open(source)
	buf := make([]byte, 1024)
	for {
		n, _ := source_file.Read(buf)
		if n == 0 {
			break
		}
		file.AddData(buf[:n])
	}
	token, _ := file.FinishWrite()
	return token
}

func downloadFile(id int) {
	// 	tool.DownLoadFile(id, "hello_re.pdf")
	tool.ReadBytes(id)
}

// func deleteFile(token string) {
// 	tool.DeleteFile(system_name, token)
// }

func test(token string) {
	// downloadFile(id)
	filename := tool.GetFileName(system_name, token)
	tool.INFO("filename: " + filename)
	// deleteFile(token)
}

// func

func main() {
	token := uploadFile()
	tool.OpenFile(system_name, token)
	startTime := time.Now()
	for i := 0; i < 1; i++ {
		test(token)
	}
	endTime := time.Now()
	fmt.Println("共计耗时：", endTime.Sub(startTime))
}
