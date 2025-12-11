import React, { useState, useEffect } from 'react';
import { Upload, FolderPlus, RefreshCw, Download, MoreVertical } from 'lucide-react';
import { fileApi } from '../services/api';
import { fileManager } from '../services/fileManager';
import { FileInfo, UploadProgress } from '../types';
import { showNotification } from './Notification';

const FileManager: React.FC = () => {
  const [files, setFiles] = useState<FileInfo[]>([]);
  const [currentPath, setCurrentPath] = useState<string>('');
  const [pathSegments, setPathSegments] = useState<string[]>([]);
  const [showUploadDialog, setShowUploadDialog] = useState(false);
  const [showCreateFolderDialog, setShowCreateFolderDialog] = useState(false);
  const [showRenameDialog, setShowRenameDialog] = useState(false);
  const [uploadProgress, setUploadProgress] = useState<UploadProgress | null>(null);
  const [selectedFile, setSelectedFile] = useState<FileInfo | null>(null);
  const [newFolderName, setNewFolderName] = useState('');
  const [newFileName, setNewFileName] = useState('');
  const [loading, setLoading] = useState(false);
  const [dropdownOpen, setDropdownOpen] = useState<string | null>(null);
  const [dropdownPosition, setDropdownPosition] = useState<{ top: number; left: number } | null>(null);

  useEffect(() => {
    loadFiles();
  }, [currentPath]);

  useEffect(() => {
    // 点击外部关闭下拉菜单
    const handleClickOutside = (event: MouseEvent) => {
      const target = event.target as Element;
      if (!target.closest('.relative')) {
        setDropdownOpen(null);
        setDropdownPosition(null);
      }
    };

    document.addEventListener('click', handleClickOutside);
    return () => document.removeEventListener('click', handleClickOutside);
  }, []);

  const loadFiles = () => {
    const fileList = fileManager.listDirectory(currentPath);
    setFiles(fileList);
    updatePathSegments();
  };

  const updatePathSegments = () => {
    const segments = currentPath.split('/').filter(segment => segment);
    setPathSegments(segments);
  };

  const handleNavigate = (path: string) => {
    setCurrentPath(path);
  };

  const handleFolderClick = (file: FileInfo) => {
    if (file.is_dir) {
      const newPath = currentPath ? `${currentPath}/${file.name}` : file.name;
      setCurrentPath(newPath);
    }
  };

  const handleFileUpload = async (file: File) => {
    if (!file) return;

    setLoading(true);
    try {
      const response = await fileApi.uploadFile(file, (progress) => {
        setUploadProgress(progress);
      });

      if (response.token) {
        // 添加到文件管理器
        const filePath = currentPath ? `${currentPath}/${file.name}` : file.name;
        fileManager.addFile(filePath, response.token, file.size);
        loadFiles();
        setShowUploadDialog(false);
        showNotification(`文件上传成功！Token: ${response.token}`, 'success');
      }
    } catch (error) {
      console.error('Upload failed:', error);
      showNotification('文件上传失败', 'error');
    } finally {
      setLoading(false);
      setUploadProgress(null);
    }
  };

  const handleFileDownload = async (file: FileInfo) => {
    if (!file.token) {
      showNotification('文件Token不存在', 'error');
      return;
    }

    try {
      const blob = await fileApi.downloadFile(file.token);
      const url = URL.createObjectURL(blob);
      const link = document.createElement('a');
      link.href = url;
      link.download = file.name;
      document.body.appendChild(link);
      link.click();
      document.body.removeChild(link);
      URL.revokeObjectURL(url);
    } catch (error) {
      console.error('Download failed:', error);
      showNotification('文件下载失败', 'error');
    }
  };

  const handleDelete = async (file: FileInfo) => {
    if (!window.confirm(`确定要删除 ${file.name} 吗？`)) {
      return;
    }

    try {
      if (file.token && !file.is_dir) {
        // 从后端删除文件
        await fileApi.deleteFile(file.token);
      }

      // 从本地文件管理器中删除
      const filePath = currentPath ? `${currentPath}/${file.name}` : file.name;
      fileManager.deleteItem(filePath);
      loadFiles();
      showNotification('删除成功', 'success');
    } catch (error) {
      console.error('Delete failed:', error);
      showNotification('删除失败', 'error');
    }
  };

  const handleCreateFolder = () => {
    if (!newFolderName.trim()) {
      showNotification('请输入文件夹名称', 'warning');
      return;
    }

    const folderPath = currentPath ? `${currentPath}/${newFolderName}` : newFolderName;
    const success = fileManager.createDirectory(folderPath);

    if (success) {
      loadFiles();
      setShowCreateFolderDialog(false);
      setNewFolderName('');
      showNotification('文件夹创建成功', 'success');
    } else {
      showNotification('文件夹创建失败', 'error');
    }
  };

  const handleRename = () => {
    if (!newFileName.trim() || !selectedFile) {
      showNotification('请输入新文件名', 'warning');
      return;
    }

    const oldPath = currentPath ? `${currentPath}/${selectedFile.name}` : selectedFile.name;
    const success = fileManager.renameItem(oldPath, newFileName);

    if (success) {
      loadFiles();
      setShowRenameDialog(false);
      setNewFileName('');
      setSelectedFile(null);
      showNotification('重命名成功', 'success');
    } else {
      showNotification('重命名失败', 'error');
    }
  };

  const formatFileSize = (bytes: number) => {
    if (bytes === 0) return '0 B';
    const k = 1024;
    const sizes = ['B', 'KB', 'MB', 'GB', 'TB'];
    const i = Math.floor(Math.log(bytes) / Math.log(k));
    return parseFloat((bytes / Math.pow(k, i)).toFixed(2)) + ' ' + sizes[i];
  };

  const handleFileInputChange = (e: React.ChangeEvent<HTMLInputElement>) => {
    const file = e.target.files?.[0];
    if (file) {
      handleFileUpload(file);
    }
  };

  return (
    <div className="file-manager">
      <div className="file-manager-card">
        {/* 面包屑导航 */}
        <div className="breadcrumb-container">
          <div className="breadcrumb">
            <button
              className="breadcrumb-button"
              onClick={() => handleNavigate('')}
              disabled={!currentPath}
            >
              ← 返回根目录
            </button>
            <span className="breadcrumb-separator">/</span>
            <button
              className="breadcrumb-button"
              onClick={() => handleNavigate('')}
            >
              根目录
            </button>
            {pathSegments.map((segment, index) => {
              const path = pathSegments.slice(0, index + 1).join('/');
              return (
                <React.Fragment key={index}>
                  <span className="breadcrumb-separator">/</span>
                  <button
                    className="breadcrumb-button"
                    onClick={() => handleNavigate(path)}
                  >
                    {segment}
                  </button>
                </React.Fragment>
              );
            })}
          </div>

          <div className="toolbar">
            <button
              className="btn btn-primary"
              onClick={() => setShowUploadDialog(true)}
            >
              <Upload size={16} />
              上传文件
            </button>
            <button
              className="btn btn-success"
              onClick={() => setShowCreateFolderDialog(true)}
            >
              <FolderPlus size={16} />
              新建文件夹
            </button>
            <button
              className="btn btn-secondary"
              onClick={loadFiles}
            >
              <RefreshCw size={16} />
              刷新
            </button>
          </div>
        </div>

        {/* 文件列表 */}
        <div className="file-list">
          <div className="file-list-header">
            <div className="file-name-header">名称</div>
            <div className="file-size-header">大小</div>
            <div className="file-date-header">修改时间</div>
            <div className="file-actions-header">操作</div>
          </div>

          {files.map((file, index) => (
            <div key={index} className={`file-row ${file.is_dir ? 'folder' : 'file'}`}>
              <div
                className="file-name-cell"
                onClick={() => file.is_dir && handleFolderClick(file)}
                style={{ cursor: file.is_dir ? 'pointer' : 'default' }}
              >
                <span className="file-icon">
                  {file.is_dir ? '📁' : '📄'}
                </span>
                <span className="file-name">{file.name}</span>
                {file.is_dir && (
                  <span className="folder-tag">文件夹</span>
                )}
              </div>
              <div className="file-size">
                {file.file_size ? formatFileSize(file.file_size) : '-'}
              </div>
              <div className="file-date">-</div>
              <div className="file-actions">
                {!file.is_dir && (
                  <button
                    className="btn btn-sm btn-primary"
                    onClick={() => handleFileDownload(file)}
                  >
                    <Download size={14} />
                    下载
                  </button>
                )}
                <div className="relative">
                  <button
                    className="btn btn-sm btn-secondary"
                    onClick={(e) => {
                      e.stopPropagation();
                      const button = e.currentTarget;
                      const rect = button.getBoundingClientRect();

                      if (dropdownOpen === file.name) {
                        setDropdownOpen(null);
                        setDropdownPosition(null);
                      } else {
                        setDropdownOpen(file.name);
                        // 计算下拉菜单位置
                        const dropdownWidth = 120;
                        const dropdownHeight = 80; // 预估下拉菜单高度
                        const margin = 4;

                        let left = rect.right - dropdownWidth;
                        let top = rect.bottom + margin;

                        // 确保不超出屏幕右边界
                        if (left + dropdownWidth > window.innerWidth) {
                          left = window.innerWidth - dropdownWidth - margin;
                        }

                        // 确保不超出屏幕左边界
                        if (left < margin) {
                          left = margin;
                        }

                        // 确保不超出屏幕底部
                        if (top + dropdownHeight > window.innerHeight) {
                          top = rect.top - dropdownHeight - margin;
                        }

                        // 确保不超出屏幕顶部
                        if (top < margin) {
                          top = margin;
                        }

                        setDropdownPosition({ top, left });
                      }
                    }}
                  >
                    <MoreVertical size={14} />
                  </button>
                  {dropdownOpen === file.name && dropdownPosition && (
                    <div
                      className="dropdown-menu"
                      style={{
                        top: `${dropdownPosition.top}px`,
                        left: `${dropdownPosition.left}px`
                      }}
                      onClick={(e) => e.stopPropagation()}
                    >
                      <button
                        className="dropdown-item"
                        onClick={() => {
                          setSelectedFile(file);
                          setNewFileName(file.name);
                          setShowRenameDialog(true);
                          setDropdownOpen(null);
                          setDropdownPosition(null);
                        }}
                      >
                        重命名
                      </button>
                      <button
                        className="dropdown-item danger"
                        onClick={() => {
                          handleDelete(file);
                          setDropdownOpen(null);
                          setDropdownPosition(null);
                        }}
                      >
                        删除
                      </button>
                    </div>
                  )}
                </div>
              </div>
            </div>
          ))}

          {files.length === 0 && (
            <div className="empty-state">
              <p>此文件夹为空</p>
            </div>
          )}
        </div>
      </div>

      {/* 上传对话框 */}
      {showUploadDialog && (
        <div className="modal-overlay" onClick={() => !loading && setShowUploadDialog(false)}>
          <div className="modal" onClick={(e) => e.stopPropagation()}>
            <div className="modal-header">
              <h3>上传文件</h3>
              <button
                className="modal-close"
                onClick={() => setShowUploadDialog(false)}
                disabled={loading}
              >
                ×
              </button>
            </div>
            <div className="modal-body">
              <div className="upload-area">
                <input
                  type="file"
                  onChange={handleFileInputChange}
                  disabled={loading}
                  style={{ display: 'none' }}
                  id="file-upload"
                />
                <label htmlFor="file-upload" className="upload-label">
                  <div className="upload-icon">📁</div>
                  <p>点击选择文件上传</p>
                </label>
              </div>

              {uploadProgress && (
                <div className="upload-progress">
                  <p>上传进度: {uploadProgress.percentage}%</p>
                  <div className="progress-bar">
                    <div
                      className="progress-fill"
                      style={{ width: `${uploadProgress.percentage}%` }}
                    />
                  </div>
                </div>
              )}
            </div>
          </div>
        </div>
      )}

      {/* 新建文件夹对话框 */}
      {showCreateFolderDialog && (
        <div className="modal-overlay" onClick={() => setShowCreateFolderDialog(false)}>
          <div className="modal" onClick={(e) => e.stopPropagation()}>
            <div className="modal-header">
              <h3>新建文件夹</h3>
              <button className="modal-close" onClick={() => setShowCreateFolderDialog(false)}>
                ×
              </button>
            </div>
            <div className="modal-body">
              <div className="form-group">
                <label>文件夹名称</label>
                <input
                  type="text"
                  value={newFolderName}
                  onChange={(e) => setNewFolderName(e.target.value)}
                  placeholder="请输入文件夹名称"
                  className="form-input"
                />
              </div>
            </div>
            <div className="modal-footer">
              <button
                className="btn btn-secondary"
                onClick={() => setShowCreateFolderDialog(false)}
              >
                取消
              </button>
              <button className="btn btn-primary" onClick={handleCreateFolder}>
                确定
              </button>
            </div>
          </div>
        </div>
      )}

      {/* 重命名对话框 */}
      {showRenameDialog && (
        <div className="modal-overlay" onClick={() => setShowRenameDialog(false)}>
          <div className="modal" onClick={(e) => e.stopPropagation()}>
            <div className="modal-header">
              <h3>重命名</h3>
              <button className="modal-close" onClick={() => setShowRenameDialog(false)}>
                ×
              </button>
            </div>
            <div className="modal-body">
              <div className="form-group">
                <label>新名称</label>
                <input
                  type="text"
                  value={newFileName}
                  onChange={(e) => setNewFileName(e.target.value)}
                  placeholder="请输入新名称"
                  className="form-input"
                />
              </div>
            </div>
            <div className="modal-footer">
              <button
                className="btn btn-secondary"
                onClick={() => setShowRenameDialog(false)}
              >
                取消
              </button>
              <button className="btn btn-primary" onClick={handleRename}>
                确定
              </button>
            </div>
          </div>
        </div>
      )}
    </div>
  );
};

export default FileManager;