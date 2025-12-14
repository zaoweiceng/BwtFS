// BwtFS Go客户端示例
//
// 功能：
// - 获取文件系统信息
// - 上传文件（支持分块上传）
// - 下载文件
// - 删除文件
//
// API参考：
// - GET /system_size - 获取文件系统总大小
// - GET /free_size - 获取文件系统剩余空间
// - POST /upload - 上传文件（支持分块）
// - GET /{token} - 下载文件
// - DELETE /delete/{token} - 删除文件
//
// 使用方法：
// go run bwtfs_client.go <command> [options]
//
// 命令：
//   info       获取文件系统信息
//   upload     上传文件
//   download   下载文件
//   delete     删除文件
//
// 选项：
//   -url URL       BwtFS服务地址 (默认: http://localhost:9999)
//   -file FILE     要上传的文件路径（仅upload命令）
//   -token TOKEN   文件访问令牌（download和delete命令）
//   -output OUTPUT 下载文件的输出路径（仅download命令）

package main

import (
	"bufio"
	"encoding/json"
	"flag"
	"fmt"
	"io"
	"net/http"
	"os"
	"path/filepath"
	"strconv"
	"strings"
	"sync"
	"time"

	"github.com/google/uuid"
)

const (
	chunkSize = 1024 * 1024 // 1MB 分块大小
)

// BwtFSClient 是BwtFS客户端结构体
type BwtFSClient struct {
	baseURL string
	client  *http.Client
}

// SystemInfo 表示文件系统信息
type SystemInfo struct {
	TotalSize uint64 `json:"system_size"`
	FreeSize  uint64 `json:"free_size"`
	UsedSize  uint64
}

// UploadResponse 表示上传响应
type UploadResponse struct {
	Status  string `json:"status"`
	Token   string `json:"token"`
	Message string `json:"message"`
}

// NewBwtFSClient 创建一个新的BwtFS客户端
func NewBwtFSClient(baseURL string) *BwtFSClient {
	return &BwtFSClient{
		baseURL: strings.TrimSuffix(baseURL, "/"),
		client: &http.Client{
			Timeout: 30 * time.Second,
		},
	}
}

// GetSystemInfo 获取文件系统信息
func (c *BwtFSClient) GetSystemInfo() (*SystemInfo, error) {
	totalSizeURL := c.baseURL + "/system_size"
	freeSizeURL := c.baseURL + "/free_size"

	// 获取总大小
	totalResp, err := c.client.Get(totalSizeURL)
	if err != nil {
		return nil, fmt.Errorf("获取总大小失败: %w", err)
	}
	defer totalResp.Body.Close()

	if totalResp.StatusCode != http.StatusOK {
		return nil, fmt.Errorf("获取总大小失败: 状态码 %d", totalResp.StatusCode)
	}

	var totalSizeResp SystemInfo
	if err := json.NewDecoder(totalResp.Body).Decode(&totalSizeResp); err != nil {
		return nil, fmt.Errorf("解析总大小响应失败: %w", err)
	}

	// 获取剩余大小
	freeResp, err := c.client.Get(freeSizeURL)
	if err != nil {
		return nil, fmt.Errorf("获取剩余大小失败: %w", err)
	}
	defer freeResp.Body.Close()

	if freeResp.StatusCode != http.StatusOK {
		return nil, fmt.Errorf("获取剩余大小失败: 状态码 %d", freeResp.StatusCode)
	}

	var freeSizeResp SystemInfo
	if err := json.NewDecoder(freeResp.Body).Decode(&freeSizeResp); err != nil {
		return nil, fmt.Errorf("解析剩余大小响应失败: %w", err)
	}

	// 计算已用大小
	usedSize := totalSizeResp.TotalSize - freeSizeResp.FreeSize

	return &SystemInfo{
		TotalSize: totalSizeResp.TotalSize,
		FreeSize:  freeSizeResp.FreeSize,
		UsedSize:  usedSize,
	}, nil
}

// UploadFile 上传文件
func (c *BwtFSClient) UploadFile(filePath string) (string, error) {
	// 检查文件是否存在
	fileInfo, err := os.Stat(filePath)
	if err != nil {
		return "", fmt.Errorf("文件不存在: %w", err)
	}

	fileSize := fileInfo.Size()
	fileName := fileInfo.Name()
	fileID := uuid.New().String()
	totalChunks := (fileSize + chunkSize - 1) / chunkSize

	fmt.Printf("上传文件: %s\n", fileName)
	fmt.Printf("文件大小: %d bytes\n", fileSize)
	fmt.Printf("总分块数: %d\n", totalChunks)

	// 打开文件
	file, err := os.Open(filePath)
	if err != nil {
		return "", fmt.Errorf("打开文件失败: %w", err)
	}
	defer file.Close()

	// 创建进度条
	var uploadedChunks int64
	var mu sync.Mutex
	var wg sync.WaitGroup

	// 模拟进度条
	go func() {
		for {
			mu.Lock()
			current := uploadedChunks
			mu.Unlock()

			if current >= int64(totalChunks) {
				break
			}

			percent := float64(current) / float64(totalChunks) * 100
			fmt.Printf("\r上传进度: %.1f%% (%d/%d)", percent, current, totalChunks)
			time.Sleep(100 * time.Millisecond)
		}
		fmt.Printf("\r上传进度: 100.0%% (%d/%d)\n", totalChunks, totalChunks)
	}()

	// 读取并上传每个分块
	var lastResponseBody []byte
	for chunkIndex := int64(0); chunkIndex < int64(totalChunks); chunkIndex++ {
		wg.Add(1)

		go func(index int64) {
			defer wg.Done()

			// 计算分块位置
			start := index * chunkSize
			end := start + chunkSize
			if end > fileSize {
				end = fileSize
			}

			// 读取分块数据
			buf := make([]byte, end-start)
			_, err := file.ReadAt(buf, start)
			if err != nil {
				fmt.Printf("\n读取分块 %d 失败: %v\n", index, err)
				os.Exit(1)
			}

			// 创建请求
			req, err := http.NewRequest("POST", c.baseURL+"/upload", strings.NewReader(string(buf)))
			if err != nil {
				fmt.Printf("\n创建请求失败: %v\n", err)
				os.Exit(1)
			}

			// 设置请求头
			req.Header.Set("Content-Type", "application/octet-stream")
			req.Header.Set("X-File-Id", fileID)
			req.Header.Set("X-Chunk-Index", strconv.FormatInt(index, 10))
			req.Header.Set("X-Total-Chunks", strconv.FormatInt(int64(totalChunks), 10))
			req.Header.Set("X-File-Size", strconv.FormatInt(fileSize, 10))
			req.Header.Set("X-File-Type", "") // 简化处理，不传递文件类型

			// 发送请求
			resp, err := c.client.Do(req)
			if err != nil {
				fmt.Printf("\n发送请求失败: %v\n", err)
				os.Exit(1)
			}
			defer resp.Body.Close()

			if resp.StatusCode != http.StatusOK {
				bodyBytes, _ := io.ReadAll(resp.Body)
				fmt.Printf("\n上传分块 %d 失败: 状态码 %d, 响应: %s\n", index, resp.StatusCode, string(bodyBytes))
				os.Exit(1)
			}

			// 保存最后一个响应的内容
			if index == int64(totalChunks)-1 {
				// 读取并保存最后一个响应的Body
				bodyBytes, readErr := io.ReadAll(resp.Body)
				if readErr != nil {
					fmt.Printf("\n读取最后一个响应失败: %v\n", readErr)
					os.Exit(1)
				}
				lastResponseBody = bodyBytes
			} else {
				// 读取其他响应体以允许连接重用
				_, _ = io.ReadAll(resp.Body)
			}

			// 更新进度
			mu.Lock()
			uploadedChunks++
			mu.Unlock()
		}(chunkIndex)

		// 限制并发数量
		if chunkIndex%10 == 9 {
			wg.Wait()
		}
	}

	wg.Wait()

	// 处理最后一个响应
	if lastResponseBody == nil {
		return "", fmt.Errorf("没有收到任何响应")
	}

	// 使用保存的响应体
	bodyBytes := lastResponseBody

	// 解析响应
	var resp UploadResponse
	if err := json.Unmarshal(bodyBytes, &resp); err != nil {
		return "", fmt.Errorf("解析响应失败: %w", err)
	}

	if resp.Status != "success" {
		return "", fmt.Errorf("上传失败: %s", resp.Message)
	}

	fmt.Println("上传成功!")
	fmt.Printf("文件令牌: %s\n", resp.Token)

	return resp.Token, nil
}

// DownloadFile 下载文件
func (c *BwtFSClient) DownloadFile(token, outputPath string) error {
	url := c.baseURL + "/" + token

	// 发送请求
	resp, err := c.client.Get(url)
	if err != nil {
		return fmt.Errorf("发送请求失败: %w", err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		return fmt.Errorf("下载失败: 状态码 %d", resp.StatusCode)
	}

	// 获取文件大小
	fileSize, err := strconv.ParseInt(resp.Header.Get("Content-Length"), 10, 64)
	if err != nil {
		fileSize = -1
	}

	fmt.Println("下载文件...")
	if fileSize > 0 {
		fmt.Printf("文件大小: %d bytes\n", fileSize)
	}

	// 创建输出目录
	outputDir := filepath.Dir(outputPath)
	if err := os.MkdirAll(outputDir, 0755); err != nil {
		return fmt.Errorf("创建输出目录失败: %w", err)
	}

	// 创建输出文件
	outFile, err := os.Create(outputPath)
	if err != nil {
		return fmt.Errorf("创建输出文件失败: %w", err)
	}
	defer outFile.Close()

	// 创建缓冲写入器
	writer := bufio.NewWriter(outFile)
	defer writer.Flush()

	// 创建进度条
	var downloadedBytes int64
	var mu sync.Mutex
	// var wg sync.WaitGroup

	// 模拟进度条
	go func() {
		for {
			mu.Lock()
			current := downloadedBytes
			mu.Unlock()

			if fileSize > 0 {
				percent := float64(current) / float64(fileSize) * 100
				fmt.Printf("\r下载进度: %.1f%% (%d/%d)", percent, current, fileSize)
			} else {
				fmt.Printf("\r下载进度: %d bytes", current)
			}

			time.Sleep(100 * time.Millisecond)

			// 检查是否下载完成
			if fileSize > 0 && current >= fileSize {
				break
			}
		}
	}()

	// 读取并写入数据
	buffer := make([]byte, chunkSize)
	for {
		n, err := resp.Body.Read(buffer)
		if err != nil && err != io.EOF {
			return fmt.Errorf("读取响应失败: %w", err)
		}

		if n == 0 {
			break
		}

		if _, err := writer.Write(buffer[:n]); err != nil {
			return fmt.Errorf("写入文件失败: %w", err)
		}

		// 更新进度
		mu.Lock()
		downloadedBytes += int64(n)
		mu.Unlock()
	}

	// 确保进度条显示100%
	if fileSize > 0 {
		fmt.Printf("\r下载进度: 100.0%% (%d/%d)\n", fileSize, fileSize)
	} else {
		fmt.Printf("\r下载进度: %d bytes\n", downloadedBytes)
	}

	fmt.Println("下载成功!")
	fmt.Printf("文件保存到: %s\n", outputPath)

	return nil
}

// DeleteFile 删除文件
func (c *BwtFSClient) DeleteFile(token string) error {
	url := c.baseURL + "/delete/" + token

	// 创建DELETE请求
	req, err := http.NewRequest("DELETE", url, nil)
	if err != nil {
		return fmt.Errorf("创建请求失败: %w", err)
	}

	// 发送请求
	resp, err := c.client.Do(req)
	if err != nil {
		return fmt.Errorf("发送请求失败: %w", err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		bodyBytes, _ := io.ReadAll(resp.Body)
		return fmt.Errorf("删除失败: 状态码 %d, 响应: %s", resp.StatusCode, string(bodyBytes))
	}

	fmt.Println("删除成功!")

	return nil
}

func main() {
	// 解析命令行参数
	var (
		url    string
		file   string
		token  string
		output string
	)

	flag.StringVar(&url, "url", "http://localhost:9999", "BwtFS服务地址")
	flag.StringVar(&file, "file", "", "要上传的文件路径")
	flag.StringVar(&token, "token", "", "文件访问令牌")
	flag.StringVar(&output, "output", "", "下载文件的输出路径")

	// 自定义flag.Usage
	flag.Usage = func() {
		fmt.Println("BwtFS Go客户端使用方法：")
		fmt.Println("  go run bwtfs_client.go <command> [options]")
		fmt.Println("  ")
		fmt.Println("命令：")
		fmt.Println("  info       获取文件系统信息")
		fmt.Println("  upload     上传文件")
		fmt.Println("  download   下载文件")
		fmt.Println("  delete     删除文件")
		fmt.Println("  ")
		fmt.Println("选项：")
		flag.PrintDefaults()
	}

	// 找到命令的位置
	commandIndex := -1
	for i, arg := range os.Args[1:] {
		// 命令不能以-开头
		if !strings.HasPrefix(arg, "-") {
			// 检查前一个参数是否是flag
			prevArgIsFlag := false
			if i > 0 {
				prevArgIsFlag = strings.HasPrefix(os.Args[1:][i-1], "-")
			}
			// 如果前一个参数不是flag，或者这是第一个参数，那么这就是命令
			if !prevArgIsFlag {
				commandIndex = i + 1 // +1是因为os.Args[0]是程序名
				break
			}
		}
	}

	// 如果找到了命令，将命令和参数分开处理
	command := ""
	if commandIndex > 0 && commandIndex < len(os.Args) {
		command = os.Args[commandIndex]
		// 创建临时的args，将选项参数放在前面
		var tempArgs []string
		tempArgs = append(tempArgs, os.Args[0]) // 程序名

		// 添加所有选项参数（以-开头的）
		for i := 1; i < len(os.Args); i++ {
			if i != commandIndex && strings.HasPrefix(os.Args[i], "-") {
				tempArgs = append(tempArgs, os.Args[i])
				// 如果这个选项有值，也一起添加
				if i+1 < len(os.Args) && !strings.HasPrefix(os.Args[i+1], "-") {
					tempArgs = append(tempArgs, os.Args[i+1])
				}
			}
		}

		// 设置os.Args为新的临时args
		os.Args = tempArgs
	}

	// 解析参数
	flag.Parse()

	// 如果没找到命令，检查flag.Args()
	if command == "" {
		if len(flag.Args()) > 0 {
			command = flag.Args()[0]
		} else {
			flag.Usage()
			os.Exit(1)
		}
	}

	// 创建客户端
	client := NewBwtFSClient(url)
	fmt.Println("请求地址：" + url)

	// 执行命令
	switch command {
	case "info":
		// 获取文件系统信息
		info, err := client.GetSystemInfo()
		if err != nil {
			fmt.Printf("获取文件系统信息失败: %v\n", err)
			os.Exit(1)
		}

		fmt.Println("\nBwtFS文件系统信息：")
		fmt.Printf("总大小: %d bytes (%.2f MB)\n", info.TotalSize, float64(info.TotalSize)/(1024*1024))
		fmt.Printf("已用大小: %d bytes (%.2f MB)\n", info.UsedSize, float64(info.UsedSize)/(1024*1024))
		fmt.Printf("剩余大小: %d bytes (%.2f MB)\n", info.FreeSize, float64(info.FreeSize)/(1024*1024))

	case "upload":
		// 上传文件
		if file == "" {
			fmt.Println("upload命令需要指定文件路径，请使用 -file 参数")
			os.Exit(1)
		}

		if _, err := client.UploadFile(file); err != nil {
			fmt.Printf("上传文件失败: %v\n", err)
			os.Exit(1)
		}

	case "download":
		// 下载文件
		if token == "" {
			fmt.Println("download命令需要指定文件令牌，请使用 -token 参数")
			os.Exit(1)
		}

		if output == "" {
			fmt.Println("download命令需要指定输出路径，请使用 -output 参数")
			os.Exit(1)
		}

		if err := client.DownloadFile(token, output); err != nil {
			fmt.Printf("下载文件失败: %v\n", err)
			os.Exit(1)
		}

	case "delete":
		// 删除文件
		if token == "" {
			fmt.Println("delete命令需要指定文件令牌，请使用 -token 参数")
			os.Exit(1)
		}

		if err := client.DeleteFile(token); err != nil {
			fmt.Printf("删除文件失败: %v\n", err)
			os.Exit(1)
		}

	default:
		fmt.Printf("未知命令: %s\n", command)
		fmt.Println("可用命令: info, upload, download, delete")
		os.Exit(1)
	}
}
