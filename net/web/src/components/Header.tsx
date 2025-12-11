import React, { useState, useEffect } from 'react';
import { FolderPlus, Download } from 'lucide-react';
import { fileApi } from '../services/api';
import { fileManager } from '../services/fileManager';
import { SystemInfo } from '../types';

const Header: React.FC = () => {
  const [systemInfo, setSystemInfo] = useState<SystemInfo | null>(null);
  const [showImportDialog, setShowImportDialog] = useState(false);

  useEffect(() => {
    loadSystemInfo();
  }, []);

  const loadSystemInfo = async () => {
    try {
      const info = await fileApi.getSystemInfo();
      setSystemInfo(info);
    } catch (error) {
      console.error('Failed to load system info:', error);
    }
  };

  const handleExportStructure = () => {
    try {
      const structure = fileManager.exportStructure();
      const blob = new Blob([structure], { type: 'application/json' });
      const url = URL.createObjectURL(blob);
      const link = document.createElement('a');
      link.href = url;
      link.download = 'filesystem_structure.json';
      document.body.appendChild(link);
      link.click();
      document.body.removeChild(link);
      URL.revokeObjectURL(url);
    } catch (error) {
      console.error('Failed to export structure:', error);
    }
  };

  const handleImportStructure = (file: File) => {
    const reader = new FileReader();
    reader.onload = (e) => {
      try {
        const content = e.target?.result as string;
        const success = fileManager.importStructure(content);
        if (success) {
          alert('文件结构导入成功');
          setShowImportDialog(false);
          // 重新加载页面以刷新文件列表
          window.location.reload();
        } else {
          alert('文件结构导入失败');
        }
      } catch (error) {
        console.error('Failed to import structure:', error);
        alert('文件格式错误，导入失败');
      }
    };
    reader.readAsText(file);
  };

  const formatBytes = (bytes: number) => {
    if (bytes === 0) return '0 B';
    const k = 1024;
    const sizes = ['B', 'KB', 'MB', 'GB', 'TB'];
    const i = Math.floor(Math.log(bytes) / Math.log(k));
    return parseFloat((bytes / Math.pow(k, i)).toFixed(2)) + ' ' + sizes[i];
  };

  const usedPercentage = systemInfo ? Math.round(((systemInfo.total_size - systemInfo.free_size) / systemInfo.total_size) * 100) : 0;
  const progressColor = usedPercentage > 90 ? '#f56c6c' : usedPercentage > 70 ? '#e6a23c' : '#67c23a';

  const handleFileInputChange = (e: React.ChangeEvent<HTMLInputElement>) => {
    const file = e.target.files?.[0];
    if (file) {
      handleImportStructure(file);
    }
  };

  return (
    <div className="header">
      <div className="header-card">
        <div className="header-content">
          <div className="logo-section">
            <h1 className="app-title">BwtFS 网盘</h1>
            <p className="app-subtitle">隐私保护的文件存储系统</p>
          </div>

          <div className="storage-info">
            <div className="stats-grid">
              <div className="stat-item">
                <div className="stat-label">总空间</div>
                <div className="stat-value">{formatBytes(systemInfo?.total_size || 0)}</div>
              </div>
              <div className="stat-item">
                <div className="stat-label">已使用</div>
                <div className="stat-value">{formatBytes((systemInfo?.total_size || 0) - (systemInfo?.free_size || 0))}</div>
              </div>
              <div className="stat-item">
                <div className="stat-label">可用空间</div>
                <div className="stat-value">{formatBytes(systemInfo?.free_size || 0)}</div>
              </div>
              <div className="stat-item">
                <div className="stat-label">使用率</div>
                <div className="stat-value">{usedPercentage}%</div>
              </div>
            </div>
            <div className="progress-container">
              <div className="progress-bar">
                <div
                  className="progress-fill"
                  style={{
                    width: `${usedPercentage}%`,
                    backgroundColor: progressColor
                  }}
                />
              </div>
            </div>
          </div>

          <div className="header-actions">
            <button
              className="btn btn-primary"
              onClick={() => setShowImportDialog(true)}
            >
              <FolderPlus size={16} />
              导入结构
            </button>
            <button
              className="btn btn-success"
              onClick={handleExportStructure}
            >
              <Download size={16} />
              导出结构
            </button>
          </div>
        </div>
      </div>

      {/* 导入结构对话框 */}
      {showImportDialog && (
        <div className="modal-overlay" onClick={() => setShowImportDialog(false)}>
          <div className="modal" onClick={(e) => e.stopPropagation()}>
            <div className="modal-header">
              <h3>导入文件结构</h3>
              <button className="modal-close" onClick={() => setShowImportDialog(false)}>
                ×
              </button>
            </div>
            <div className="modal-body">
              <div className="upload-area">
                <input
                  type="file"
                  accept=".json"
                  onChange={handleFileInputChange}
                  style={{ display: 'none' }}
                  id="json-upload"
                />
                <label htmlFor="json-upload" className="upload-label">
                  <div className="upload-icon">📁</div>
                  <p>点击或拖拽JSON文件到此处上传</p>
                  <p className="upload-tip">仅支持JSON格式的文件结构</p>
                </label>
              </div>
            </div>
          </div>
        </div>
      )}
    </div>
  );
};

export default Header;