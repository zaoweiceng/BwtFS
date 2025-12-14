import React, { useState } from 'react';
import { FolderPlus, Download, Trash, Plus } from 'lucide-react';
import { fileApi } from '../services/api';
import { fileManager } from '../services/fileManager';
import { useSystemInfo } from '../contexts/SystemInfoContext';
import { showNotification } from './Notification';

const Header: React.FC = () => {
  const { systemInfo, loadSystemInfo, isLoading } = useSystemInfo();
  const [showImportDialog, setShowImportDialog] = useState(false);
  const [showClearDialog, setShowClearDialog] = useState(false);
  const [showAddFileDialog, setShowAddFileDialog] = useState(false);
  const [fileToken, setFileToken] = useState('');
  const [fileName, setFileName] = useState('');
  const [filePath, setFilePath] = useState('');
  const [addToDirectory, setAddToDirectory] = useState(true);
  const [isImportDragOver, setIsImportDragOver] = useState(false);

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
          showNotification('文件目录导入成功', 'success');
          setShowImportDialog(false);
          // 重新加载页面以刷新文件列表
          window.location.reload();
        } else {
          showNotification('文件目录导入失败', 'error');
        }
      } catch (error) {
        console.error('Failed to import structure:', error);
        showNotification('文件格式错误，导入失败', 'error');
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

  const handleClearDirectory = () => {
    try {
      const success = fileManager.clearDirectory();
      if (success) {
        showNotification('目录清空成功', 'success');
        setShowClearDialog(false);
        // 重新加载页面以刷新文件列表
        window.location.reload();
      } else {
        showNotification('目录清空失败', 'error');
      }
    } catch (error) {
      console.error('Failed to clear directory:', error);
      showNotification('目录清空失败', 'error');
    }
  };

  const handleAddFile = async () => {
    if (!fileToken.trim()) {
      showNotification('请输入文件 Token', 'warning');
      return;
    }

    if (!fileName.trim()) {
      showNotification('请输入文件名', 'warning');
      return;
    }

    try {
      // 下载文件内容
      const blob = await fileApi.downloadFile(fileToken.trim());

      if (addToDirectory) {
        // 添加到文件系统
        const fullFilePath = filePath.trim() ? `${filePath.trim()}/${fileName.trim()}` : fileName.trim();
        const success = fileManager.addFile(fullFilePath, fileToken.trim(), blob.size);

        if (success) {
          showNotification(`文件 ${fileName.trim()} 添加成功`, 'success');
          // 更新系统信息
          await loadSystemInfo();
          setShowAddFileDialog(false);
          // 重置表单
          setFileToken('');
          setFileName('');
          setFilePath('');
          setAddToDirectory(true);
        } else {
          showNotification('文件添加失败', 'error');
        }
      } else {
        // 仅下载到本地
        const url = URL.createObjectURL(blob);
        const link = document.createElement('a');
        link.href = url;
        link.download = fileName.trim();
        document.body.appendChild(link);
        link.click();
        document.body.removeChild(link);
        URL.revokeObjectURL(url);

        showNotification(`文件 ${fileName.trim()} 下载成功`, 'success');
        setShowAddFileDialog(false);
        // 重置表单
        setFileToken('');
        setFileName('');
        setFilePath('');
        setAddToDirectory(true);
      }
    } catch (error) {
      console.error('Failed to add/download file:', error);
      showNotification('文件 Token 无效或下载失败', 'error');
    }
  };

  const handleOpenAddFileDialog = () => {
    // 设置默认值
    setFileName('new_file');
    setFilePath('');
    setAddToDirectory(true);
    setShowAddFileDialog(true);
  };

  const handleFileInputChange = (e: React.ChangeEvent<HTMLInputElement>) => {
    const file = e.target.files?.[0];
    if (file) {
      handleImportStructure(file);
    }
  };

  const handleImportDragOver = (e: React.DragEvent) => {
    e.preventDefault();
    e.stopPropagation();
    setIsImportDragOver(true);
  };

  const handleImportDragLeave = (e: React.DragEvent) => {
    e.preventDefault();
    e.stopPropagation();
    setIsImportDragOver(false);
  };

  const handleImportDrop = (e: React.DragEvent) => {
    e.preventDefault();
    e.stopPropagation();
    setIsImportDragOver(false);

    const files = e.dataTransfer.files;
    if (files && files.length > 0) {
      const file = files[0];
      // 检查是否为 JSON 文件
      if (file.name.endsWith('.json')) {
        handleImportStructure(file);
      } else {
        showNotification('仅支持 JSON 格式的文件目录', 'error');
      }
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
              导入目录
            </button>
            <button
              className="btn btn-success"
              onClick={handleExportStructure}
            >
              <Download size={16} />
              导出目录
            </button>
            <button
              className="btn btn-danger"
              onClick={() => setShowClearDialog(true)}
            >
              <Trash size={16} />
              清空目录
            </button>
            <button
              className="btn btn-info"
              onClick={handleOpenAddFileDialog}
            >
              <Plus size={16} />
              添加文件
            </button>
          </div>
        </div>
      </div>

      {/* 导入目录对话框 */}
      {showImportDialog && (
        <div className="modal-overlay" onClick={() => setShowImportDialog(false)}>
          <div className="modal" onClick={(e) => e.stopPropagation()}>
            <div className="modal-header">
              <h3>导入文件目录</h3>
              <button className="modal-close" onClick={() => setShowImportDialog(false)}>
                ×
              </button>
            </div>
            <div className="modal-body">
              <div
                className={`upload-area ${isImportDragOver ? 'drag-over' : ''}`}
                onDragOver={handleImportDragOver}
                onDragLeave={handleImportDragLeave}
                onDrop={handleImportDrop}
              >
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
                  <p className="upload-tip">仅支持JSON格式的文件目录</p>
                </label>
              </div>
            </div>
          </div>
        </div>
      )}

      {/* 清空目录确认对话框 */}
      {showClearDialog && (
        <div className="modal-overlay" onClick={() => setShowClearDialog(false)}>
          <div className="modal" onClick={(e) => e.stopPropagation()}>
            <div className="modal-header">
              <h3>清空目录</h3>
              <button className="modal-close" onClick={() => setShowClearDialog(false)}>
                ×
              </button>
            </div>
            <div className="modal-body">
              <div className="warning-message">
                <div className="warning-icon">⚠️</div>
                <p><strong>警告：此操作将清空整个目录结构！</strong></p>
                <p>所有文件和文件夹都将被删除，此操作不可撤销。</p>
                <p>请确认您是否要继续清空目录？</p>
              </div>
            </div>
            <div className="modal-footer">
              <button
                className="btn btn-secondary"
                onClick={() => setShowClearDialog(false)}
              >
                取消
              </button>
              <button
                className="btn btn-danger"
                onClick={handleClearDirectory}
              >
                确认清空
              </button>
            </div>
          </div>
        </div>
      )}

      {/* 添加文件对话框 */}
      {showAddFileDialog && (
        <div className="modal-overlay" onClick={() => setShowAddFileDialog(false)}>
          <div className="modal" onClick={(e) => e.stopPropagation()}>
            <div className="modal-header">
              <h3>添加文件</h3>
              <button className="modal-close" onClick={() => setShowAddFileDialog(false)}>
                ×
              </button>
            </div>
            <div className="modal-body">
              <div className="form-group">
                <label>文件 Token *</label>
                <input
                  type="text"
                  value={fileToken}
                  onChange={(e) => setFileToken(e.target.value)}
                  placeholder="请输入文件的访问 Token"
                  className="form-input"
                />
              </div>
              <div className="form-group">
                <label>文件名 *</label>
                <input
                  type="text"
                  value={fileName}
                  onChange={(e) => setFileName(e.target.value)}
                  placeholder="请输入文件名（包含扩展名）"
                  className="form-input"
                />
              </div>
              <div className="form-group">
                <label>
                  <input
                    type="checkbox"
                    checked={addToDirectory}
                    onChange={(e) => setAddToDirectory(e.target.checked)}
                  />
                  添加到文件系统
                </label>
                <p className="form-help">取消勾选将仅下载文件到本地</p>
              </div>
              {addToDirectory && (
                <div className="form-group">
                  <label>文件路径</label>
                  <input
                    type="text"
                    value={filePath}
                    onChange={(e) => setFilePath(e.target.value)}
                    placeholder="留空保存到根目录，或输入路径如：documents/images"
                    className="form-input"
                  />
                  <p className="form-help">支持多级目录，路径分隔符使用 /</p>
                </div>
              )}
            </div>
            <div className="modal-footer">
              <button
                className="btn btn-secondary"
                onClick={() => setShowAddFileDialog(false)}
              >
                取消
              </button>
              <button
                className="btn btn-primary"
                onClick={handleAddFile}
              >
                {addToDirectory ? '添加文件' : '下载文件'}
              </button>
            </div>
          </div>
        </div>
      )}
    </div>
  );
};

export default Header;