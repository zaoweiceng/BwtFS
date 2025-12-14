#!/usr/bin/env python3
"""
BwtFS Python客户端示例

功能：
- 文件上传（支持分块上传）
- 文件下载
- 文件删除
- 获取文件系统信息

API参考：
- GET /system_size - 获取文件系统总大小
- GET /free_size - 获取文件系统剩余空间
- POST /upload - 上传文件（支持分块）
- GET /{token} - 下载文件
- DELETE /delete/{token} - 删除文件

使用方法：
python bwtfs_client.py <command> [options]

命令：
  info       获取文件系统信息
  upload     上传文件
  download   下载文件
  delete     删除文件

选项：
  -h, --help     显示帮助信息
  -u URL, --url URL  BwtFS服务地址 (默认: http://localhost:9999)
  -f FILE, --file FILE  要上传的文件路径（仅upload命令）
  -t TOKEN, --token TOKEN  文件访问令牌（download和delete命令）
  -o OUTPUT, --output OUTPUT  下载文件的输出路径（仅download命令）
"""

import requests
import argparse
import os
import uuid
import json
from tqdm import tqdm

CHUNK_SIZE = 1024 * 1024  # 1MB 分块大小


class BwtFSClient:
    def __init__(self, base_url):
        """初始化BwtFS客户端
        
        Args:
            base_url: BwtFS服务地址
        """
        self.base_url = base_url.rstrip('/')
        self.session = requests.Session()
        self.session.headers.update({
            'Connection': 'keep-alive'
        })
    
    def get_system_info(self):
        """获取文件系统信息
        
        Returns:
            dict: 包含总大小和剩余空间的字典
        """
        try:
            total_size_response = self.session.get(f'{self.base_url}/system_size')
            free_size_response = self.session.get(f'{self.base_url}/free_size')
            
            if total_size_response.status_code == 200 and free_size_response.status_code == 200:
                total_size = total_size_response.json()['system_size']
                free_size = free_size_response.json()['free_size']
                
                return {
                    'total_size': total_size,
                    'free_size': free_size,
                    'used_size': total_size - free_size
                }
            else:
                print(f"Error getting system info: {total_size_response.status_code} / {free_size_response.status_code}")
                return None
        except Exception as e:
            print(f"Error getting system info: {e}")
            return None
    
    def upload_file(self, file_path):
        """上传文件（支持分块上传）
        
        Args:
            file_path: 要上传的文件路径
            
        Returns:
            str: 上传成功后返回的文件访问令牌
        """
        if not os.path.exists(file_path):
            print(f"File not found: {file_path}")
            return None
        
        file_size = os.path.getsize(file_path)
        file_name = os.path.basename(file_path)
        file_id = str(uuid.uuid4())
        total_chunks = (file_size + CHUNK_SIZE - 1) // CHUNK_SIZE
        
        print(f"Uploading file: {file_name}")
        print(f"File size: {file_size} bytes")
        print(f"Total chunks: {total_chunks}")
        
        try:
            with open(file_path, 'rb') as f:
                with tqdm(total=total_chunks, unit='chunk') as pbar:
                    for chunk_index in range(total_chunks):
                        start = chunk_index * CHUNK_SIZE
                        end = min(start + CHUNK_SIZE, file_size)
                        chunk = f.read(end - start)
                        
                        headers = {
                            'Content-Type': 'application/octet-stream',
                            'X-File-Id': file_id,
                            'X-Chunk-Index': str(chunk_index),
                            'X-Total-Chunks': str(total_chunks),
                            'X-File-Size': str(file_size),
                            'X-File-Type': ''  # 简化处理，不传递文件类型
                        }
                        
                        response = self.session.post(f'{self.base_url}/upload', headers=headers, data=chunk)
                        
                        if response.status_code != 200:
                            print(f"Upload failed at chunk {chunk_index}: {response.status_code} - {response.text}")
                            return None
                        
                        pbar.update(1)
            
            # 解析最后一个响应，获取文件令牌
            response_data = response.json()
            if response_data.get('status') == 'success':
                token = response_data.get('token')
                print(f"Upload successful!")
                print(f"File token: {token}")
                return token
            else:
                print(f"Upload failed: {response_data.get('message', 'Unknown error')}")
                return None
                
        except Exception as e:
            print(f"Error uploading file: {e}")
            return None
    
    def download_file(self, token, output_path):
        """下载文件
        
        Args:
            token: 文件访问令牌
            output_path: 下载文件的输出路径
            
        Returns:
            bool: 下载是否成功
        """
        try:
            response = self.session.get(f'{self.base_url}/{token}', stream=True)
            
            if response.status_code == 200:
                file_size = int(response.headers.get('Content-Length', 0))
                
                print(f"Downloading file...")
                print(f"File size: {file_size} bytes")
                
                with open(output_path, 'wb') as f:
                    with tqdm(total=file_size, unit='B', unit_scale=True, unit_divisor=1024) as pbar:
                        for chunk in response.iter_content(chunk_size=CHUNK_SIZE):
                            if chunk:
                                f.write(chunk)
                                pbar.update(len(chunk))
                
                print(f"Download successful!")
                print(f"File saved to: {output_path}")
                return True
            else:
                print(f"Download failed: {response.status_code} - {response.text}")
                return False
                
        except Exception as e:
            print(f"Error downloading file: {e}")
            return False
    
    def delete_file(self, token):
        """删除文件
        
        Args:
            token: 文件访问令牌
            
        Returns:
            bool: 删除是否成功
        """
        try:
            response = self.session.delete(f'{self.base_url}/delete/{token}')
            
            if response.status_code == 200:
                print(f"Delete successful!")
                return True
            else:
                print(f"Delete failed: {response.status_code} - {response.text}")
                return False
                
        except Exception as e:
            print(f"Error deleting file: {e}")
            return False


def main():
    parser = argparse.ArgumentParser(description='BwtFS Python客户端示例')
    parser.add_argument('command', choices=['info', 'upload', 'download', 'delete'], help='要执行的命令')
    parser.add_argument('-u', '--url', default='http://localhost:9999', help='BwtFS服务地址')
    parser.add_argument('-f', '--file', help='要上传的文件路径（仅upload命令）')
    parser.add_argument('-t', '--token', help='文件访问令牌（download和delete命令）')
    parser.add_argument('-o', '--output', help='下载文件的输出路径（仅download命令）')
    
    args = parser.parse_args()
    
    client = BwtFSClient(args.url)
    
    if args.command == 'info':
        # 获取文件系统信息
        info = client.get_system_info()
        if info:
            print("\nBwtFS文件系统信息：")
            print(f"总大小: {info['total_size']:,} bytes ({info['total_size'] / (1024*1024):.2f} MB)")
            print(f"已用大小: {info['used_size']:,} bytes ({info['used_size'] / (1024*1024):.2f} MB)")
            print(f"剩余大小: {info['free_size']:,} bytes ({info['free_size'] / (1024*1024):.2f} MB)")
    
    elif args.command == 'upload':
        # 上传文件
        if not args.file:
            parser.error("upload命令需要指定文件路径，请使用 -f/--file 参数")
        client.upload_file(args.file)
    
    elif args.command == 'download':
        # 下载文件
        if not args.token:
            parser.error("download命令需要指定文件令牌，请使用 -t/--token 参数")
        if not args.output:
            parser.error("download命令需要指定输出路径，请使用 -o/--output 参数")
        client.download_file(args.token, args.output)
    
    elif args.command == 'delete':
        # 删除文件
        if not args.token:
            parser.error("delete命令需要指定文件令牌，请使用 -t/--token 参数")
        client.delete_file(args.token)


if __name__ == '__main__':
    main()
