import React, { useEffect } from 'react';
import './App.css';
import FileManager from './components/FileManager';
import Header from './components/Header';
import Notification from './components/Notification';
import { SystemInfoProvider } from './contexts/SystemInfoContext';
import { fileApi } from './services/api';
import { showNotification } from './components/Notification';

function App() {
  useEffect(() => {
    // 测试API连接
    const testApi = async () => {
      try {
        console.log('Testing API connection to:', process.env.REACT_APP_API_BASE_URL);
        const systemInfo = await fileApi.getSystemInfo();
        console.log('API connection successful:', systemInfo);
        showNotification('API连接成功', 'success');
      } catch (error) {
        console.error('API connection failed:', error);
        showNotification(`API连接失败: ${(error as Error).message}`, 'error');
      }
    };

    testApi();

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
        <Header />
        <main className="main-content">
          <FileManager />
        </main>
      </SystemInfoProvider>
    </div>
  );
}

export default App;
