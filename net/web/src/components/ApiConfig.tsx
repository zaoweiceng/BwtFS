import React, { useState, useEffect } from 'react';
import { Settings as SettingsIcon, X, Save, RotateCcw } from 'lucide-react';
import { showNotification } from './Notification';

interface ApiConfigProps {
  onApiUrlChange: (newUrl: string) => void;
  currentUrl: string;
}

const ApiConfig: React.FC<ApiConfigProps> = ({ onApiUrlChange, currentUrl }) => {
  const [showConfig, setShowConfig] = useState(false);
  const [apiUrl, setApiUrl] = useState(currentUrl);
  const [tempUrl, setTempUrl] = useState(currentUrl);

  useEffect(() => {
    // 从localStorage读取保存的API地址
    const savedUrl = localStorage.getItem('bwtfs_api_url');
    if (savedUrl) {
      setApiUrl(savedUrl);
      setTempUrl(savedUrl);
      onApiUrlChange(savedUrl);
    } else {
      // 如果没有保存的地址，使用环境变量中的地址
      setApiUrl(currentUrl);
      setTempUrl(currentUrl);
    }
  }, [currentUrl, onApiUrlChange]);

  const handleSave = () => {
    if (!tempUrl.trim()) {
      showNotification('API地址不能为空', 'error');
      return;
    }

    // 验证URL格式
    try {
      const url = new URL(tempUrl.trim());
      if (!['http:', 'https:'].includes(url.protocol)) {
        throw new Error('仅支持HTTP和HTTPS协议');
      }
    } catch (error) {
      showNotification('API地址格式错误，请输入有效的HTTP/HTTPS地址', 'error');
      return;
    }

    // 保存到localStorage
    localStorage.setItem('bwtfs_api_url', tempUrl.trim());
    setApiUrl(tempUrl.trim());
    onApiUrlChange(tempUrl.trim());
    setShowConfig(false);
    showNotification('后端地址已保存', 'success');
  };

  const handleReset = () => {
    setTempUrl(currentUrl);
    localStorage.removeItem('bwtfs_api_url');
    setApiUrl(currentUrl);
    onApiUrlChange(currentUrl);
    setShowConfig(false);
    showNotification('已重置为默认地址', 'success');
  };

  const handleTest = async () => {
    if (!tempUrl.trim()) {
      showNotification('请先输入API地址', 'warning');
      return;
    }

    try {
      // 测试连接
      const testUrl = `${tempUrl.trim()}/system_size`;
      const response = await fetch(testUrl, {
        method: 'GET',
        headers: {
          'Content-Type': 'application/json',
        },
      });

      if (response.ok) {
        showNotification('连接测试成功', 'success');
      } else {
        showNotification(`连接测试失败: ${response.status}`, 'error');
      }
    } catch (error) {
      showNotification('连接测试失败，请检查地址是否正确', 'error');
    }
  };

  return (
    <>
      {/* 设置按钮 */}
      <button
        className="btn btn-secondary"
        onClick={() => setShowConfig(true)}
        title={`当前后端地址: ${apiUrl}`}
      >
        <SettingsIcon size={16} />
        后端设置
      </button>

      {/* 配置对话框 */}
      {showConfig && (
        <div className="modal-overlay" onClick={() => setShowConfig(false)}>
          <div className="modal" onClick={(e) => e.stopPropagation()}>
            <div className="modal-header">
              <h3>后端地址设置</h3>
              <button className="modal-close" onClick={() => setShowConfig(false)}>
                <X size={16} />
              </button>
            </div>
            <div className="modal-body">
              <div className="form-group">
                <label>后端API地址</label>
                <input
                  type="text"
                  value={tempUrl}
                  onChange={(e) => setTempUrl(e.target.value)}
                  placeholder="例如: http://127.0.0.1:9999"
                  className="form-input"
                />
                <p className="form-help">
                  请输入后端服务的完整地址，包含协议（http://或https://）和端口号
                </p>
              </div>

              <div className="form-group">
                <label>当前地址</label>
                <div className="current-url" style={{
                  background: '#f5f5f5',
                  padding: '8px 12px',
                  borderRadius: '4px',
                  border: '1px solid #ddd',
                  fontFamily: 'monospace',
                  fontSize: '14px',
                  wordBreak: 'break-all'
                }}>
                  <code>{apiUrl}</code>
                </div>
              </div>

              <div className="form-group">
                <button
                  className="btn btn-info"
                  style={{
                    fontSize: '12px',
                    padding: '6px 12px'
                  }}
                  onClick={handleTest}
                >
                  测试连接
                </button>
              </div>
            </div>
            <div className="modal-footer">
              <button
                className="btn btn-secondary"
                onClick={handleReset}
              >
                <RotateCcw size={16} />
                重置默认
              </button>
              <button
                className="btn btn-secondary"
                onClick={() => setShowConfig(false)}
              >
                取消
              </button>
              <button
                className="btn btn-primary"
                onClick={handleSave}
              >
                <Save size={16} />
                保存
              </button>
            </div>
          </div>
        </div>
      )}

    </>
  );
};

export default ApiConfig;