import axios from 'axios';
import { FileInfo, SystemInfo, ApiResponse, UploadProgress } from '../types';

let API_BASE_URL = process.env.REACT_APP_API_BASE_URL || '';

// 创建一个函数来获取axios实例
const createApiInstance = (baseURL: string) => {
  return axios.create({
    baseURL,
    timeout: 300000, // 5分钟超时
    headers: {
      'Content-Type': 'application/json',
    },
    withCredentials: false // 禁用withCredentials以支持CORS通配符
  });
};

// 初始API实例
let api = createApiInstance(API_BASE_URL);

// 添加请求拦截器用于调试
api.interceptors.request.use(
  (config) => {
    console.log('API Request:', {
      method: config.method?.toUpperCase(),
      url: `${API_BASE_URL}${config.url}`,
      headers: config.headers,
      data: config.data
    });
    return config;
  },
  (error) => {
    console.error('API Request Error:', error);
    return Promise.reject(error);
  }
);

// 添加响应拦截器用于调试
api.interceptors.response.use(
  (response) => {
    console.log('API Response:', {
      status: response.status,
      data: response.data,
      headers: response.headers
    });
    return response;
  },
  (error) => {
    console.error('API Response Error:', {
      status: error.response?.status,
      data: error.response?.data,
      message: error.message
    });
    return Promise.reject(error);
  }
);

// 更新API基础URL的函数
export const updateApiBaseUrl = (newUrl: string) => {
  API_BASE_URL = newUrl;
  api = createApiInstance(newUrl);

  // 重新添加拦截器
  api.interceptors.request.use(
    (config) => {
      console.log('API Request:', {
        method: config.method?.toUpperCase(),
        url: `${API_BASE_URL}${config.url}`,
        headers: config.headers,
        data: config.data
      });
      return config;
    },
    (error) => {
      console.error('API Request Error:', error);
      return Promise.reject(error);
    }
  );

  api.interceptors.response.use(
    (response) => {
      console.log('API Response:', {
        status: response.status,
        data: response.data,
        headers: response.headers
      });
      return response;
    },
    (error) => {
      console.error('API Response Error:', {
        status: error.response?.status,
        data: error.response?.data,
        message: error.message
      });
      return Promise.reject(error);
    }
  );
};

// 获取当前API基础URL
export const getApiBaseUrl = () => API_BASE_URL;

export const fileApi = {
  // 获取文件系统信息
  async getSystemInfo(): Promise<SystemInfo> {
    const [systemSizeResponse, freeSizeResponse] = await Promise.all([
      api.get('/system_size'),
      api.get('/free_size')
    ]);

    return {
      file_size: systemSizeResponse.data.system_size,
      block_size: 4096,
      block_count: Math.floor(systemSizeResponse.data.system_size / 4096),
      used_size: systemSizeResponse.data.system_size - freeSizeResponse.data.free_size,
      total_size: systemSizeResponse.data.system_size,
      free_size: freeSizeResponse.data.free_size,
      create_time: Date.now(),
      modify_time: Date.now()
    };
  },

  // 下载文件
  async downloadFile(token: string): Promise<Blob> {
    const response = await api.get(`/${token}`, {
      responseType: 'blob'
    });
    return response.data;
  },

  // 上传文件（分块上传）
  async uploadFile(
    file: File,
    onProgress?: (progress: UploadProgress) => void
  ): Promise<ApiResponse> {
    const CHUNK_SIZE = 1024 * 1024; // 1MB
    const totalChunks = Math.ceil(file.size / CHUNK_SIZE);
    const fileId = `${Date.now()}-${Math.random().toString(36).substr(2, 9)}`;

    console.log('Starting file upload:', {
      fileName: file.name,
      fileSize: file.size,
      fileType: file.type,
      totalChunks,
      fileId
    });

    for (let chunkIndex = 0; chunkIndex < totalChunks; chunkIndex++) {
      const start = chunkIndex * CHUNK_SIZE;
      const end = Math.min(start + CHUNK_SIZE, file.size);
      const chunk = file.slice(start, end);

      const headers = {
        'Content-Type': 'application/octet-stream',
        'X-File-Id': fileId,
        'Connection': 'keep-alive',
        'X-Chunk-Index': chunkIndex.toString(),
        'X-Total-Chunks': totalChunks.toString(),
        'X-File-Size': file.size.toString(),
        'X-File-Type': file.type || 'application/octet-stream'
      };

      try {
        const response = await api.post('/upload', chunk, {
          headers,
          onUploadProgress: (progressEvent) => {
            if (onProgress && progressEvent.total) {
              const loaded = progressEvent.loaded + (chunkIndex * CHUNK_SIZE);
              const total = file.size;
              const percentage = Math.round((loaded / total) * 100);
              onProgress({ loaded, total, percentage });
              console.log(`Upload progress chunk ${chunkIndex + 1}/${totalChunks}: ${percentage}%`);
            }
          },
          timeout: 60000 // 60秒超时
        });

        console.log(`Chunk ${chunkIndex + 1}/${totalChunks} uploaded successfully`);

        // 最后一个分块会返回token
        if (chunkIndex + 1 === totalChunks) {
          console.log('Upload completed, response:', response.data);
          return response.data;
        }
      } catch (error) {
        console.error(`Error uploading chunk ${chunkIndex + 1}:`, error);
        throw error;
      }
    }

    throw new Error('Upload failed - no response received');
  },

  // 删除文件
  async deleteFile(token: string): Promise<ApiResponse> {
    const response = await api.delete(`/delete/${token}`);
    return response.data;
  }
};