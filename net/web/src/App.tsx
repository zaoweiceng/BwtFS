import React, { useEffect, useState } from 'react';
import './App.css';
import FileManager from './components/FileManager';
import Header from './components/Header';
import Notification from './components/Notification';
import { SystemInfoProvider } from './contexts/SystemInfoContext';
import { fileApi, updateApiBaseUrl, getApiBaseUrl } from './services/api';
import { showNotification } from './components/Notification';

function App() {
  const [currentApiUrl, setCurrentApiUrl] = useState(getApiBaseUrl());

  // 处理API地址变更
  const handleApiUrlChange = (newUrl: string) => {
    updateApiBaseUrl(newUrl);
    setCurrentApiUrl(newUrl);

    // 重新测试API连接
    testApiConnection();
  };

  const testApiConnection = async () => {
    try {
      console.log('Testing API connection to:', currentApiUrl);
      const systemInfo = await fileApi.getSystemInfo();
      console.log('API connection successful:', systemInfo);
      showNotification('API连接成功', 'success');
    } catch (error) {
      console.error('API connection failed:', error);
      showNotification(`API连接失败: ${(error as Error).message}`, 'error');
    }
  };

  useEffect(() => {
    // 测试API连接
    testApiConnection();

    // 添加全局拖拽事件处理，防止文件在页面其他地方被浏览器打开
    const handleGlobalDragOver = (e: DragEvent) => {
      e.preventDefault();
      e.stopPropagation();
    };

    const handleGlobalDrop = (e: DragEvent) => {
      e.preventDefault();
      e.stopPropagation();
    };

    document.addEventListener('dragover', handleGlobalDragOver);
    document.addEventListener('drop', handleGlobalDrop);

    // 清理事件监听器
    return () => {
      document.removeEventListener('dragover', handleGlobalDragOver);
      document.removeEventListener('drop', handleGlobalDrop);
    };
  }, []);

  return (
    <div className="app">
      <SystemInfoProvider>
        <Notification />
        <Header onApiUrlChange={handleApiUrlChange} currentApiUrl={currentApiUrl} />
        <main className="main-content">
          <FileManager />
        </main>
      </SystemInfoProvider>
    </div>
  );
}

export default App;
