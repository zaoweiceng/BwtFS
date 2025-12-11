import React, { useEffect, useState } from 'react';
import './App.css';
import FileManager from './components/FileManager';
import Header from './components/Header';
import { fileApi } from './services/api';

function App() {
  const [apiStatus, setApiStatus] = useState<string>('Testing...');

  useEffect(() => {
    // 测试API连接
    const testApi = async () => {
      try {
        console.log('Testing API connection to:', process.env.REACT_APP_API_BASE_URL);
        const systemInfo = await fileApi.getSystemInfo();
        console.log('API connection successful:', systemInfo);
        setApiStatus('Connected');
      } catch (error) {
        console.error('API connection failed:', error);
        setApiStatus('Failed: ' + (error as Error).message);
      }
    };

    testApi();
  }, []);

  return (
    <div className="app">
      {/* 开发环境下的API状态显示 */}
      {process.env.NODE_ENV === 'development' && (
        <div style={{
          position: 'fixed',
          top: '10px',
          right: '10px',
          backgroundColor: apiStatus === 'Connected' ? '#4CAF50' : '#f44336',
          color: 'white',
          padding: '8px 12px',
          borderRadius: '4px',
          fontSize: '12px',
          zIndex: 9999
        }}>
          API: {apiStatus} | Base URL: {process.env.REACT_APP_API_BASE_URL}
        </div>
      )}
      <Header />
      <main className="main-content">
        <FileManager />
      </main>
    </div>
  );
}

export default App;
