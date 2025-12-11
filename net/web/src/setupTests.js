// 简单的调试脚本
console.log('Environment variables:', {
  REACT_APP_API_BASE_URL: process.env.REACT_APP_API_BASE_URL,
  NODE_ENV: process.env.NODE_ENV
});

// 测试API连接
import { fileApi } from './services/api';

// 在开发环境下测试API连接
if (process.env.NODE_ENV === 'development') {
  setTimeout(async () => {
    try {
      console.log('Testing API connection...');
      const systemInfo = await fileApi.getSystemInfo();
      console.log('API test successful:', systemInfo);
    } catch (error) {
      console.error('API test failed:', error);
    }
  }, 2000);
}