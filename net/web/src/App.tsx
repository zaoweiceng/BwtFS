import React, { useEffect } from 'react';
import './App.css';
import FileManager from './components/FileManager';
import Header from './components/Header';
import Notification from './components/Notification';
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
  }, []);

  return (
    <div className="app">
      <Notification />
      <Header />
      <main className="main-content">
        <FileManager />
      </main>
    </div>
  );
}

export default App;
