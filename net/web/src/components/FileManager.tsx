import React, { useState, useEffect } from 'react';
import { Upload, FolderPlus, RefreshCw, Download, MoreVertical, Eye, Search, X, Move } from 'lucide-react';
import { fileApi } from '../services/api';
import { fileManager } from '../services/fileManager';
import { FileInfo, UploadProgress } from '../types';
import { showNotification } from './Notification';
import FilePreview from './FilePreview';


const FileManager: React.FC = () => {
  const [files, setFiles] = useState<FileInfo[]>([]);
  const [allFiles, setAllFiles] = useState<FileInfo[]>([]); // 存储所有文件用于搜索
  const [currentPath, setCurrentPath] = useState<string>('');
  const [pathSegments, setPathSegments] = useState<string[]>([]);
  const [showUploadDialog, setShowUploadDialog] = useState(false);
  const [showCreateFolderDialog, setShowCreateFolderDialog] = useState(false);
  const [showRenameDialog, setShowRenameDialog] = useState(false);
  const [showMoveDialog, setShowMoveDialog] = useState(false);
  const [uploadProgress, setUploadProgress] = useState<UploadProgress | null>(null);
  const [selectedFile, setSelectedFile] = useState<FileInfo | null>(null);
  const [newFolderName, setNewFolderName] = useState('');
  const [newFileName, setNewFileName] = useState('');
  const [loading, setLoading] = useState(false);
  const [showPreview, setShowPreview] = useState(false);
  const [previewFile, setPreviewFile] = useState<FileInfo | null>(null);
  const [searchQuery, setSearchQuery] = useState('');
  const [isSearching, setIsSearching] = useState(false);
  const [availableFolders, setAvailableFolders] = useState<string[]>([]);
  const [selectedTargetPath, setSelectedTargetPath] = useState<string>(''); // Move dialog target
  const [expandedFolders, setExpandedFolders] = useState<Set<string>>(new Set()); // Track expanded folders in tree

  useEffect(() => {
    loadFiles();
  }, [currentPath]);

  const loadFiles = () => {
    const fileList = fileManager.listDirectory(currentPath);
    setFiles(fileList);
    setAllFiles(fileList); // 保存所有文件用于搜索
    updatePathSegments();
  };

  const handleSearch = (query: string) => {
    setSearchQuery(query);
    setIsSearching(query.trim() !== '');

    if (query.trim() === '') {
      // 搜索为空时显示当前目录文件
      setFiles(allFiles);
      return;
    }

    // 使用递归搜索所有文件和文件夹
    const searchResults = fileManager.searchFiles(query, currentPath);
    setFiles(searchResults);
  };

  const clearSearch = () => {
    setSearchQuery('');
    setIsSearching(false);
    // 清除搜索时重新加载当前目录的文件
    loadFiles();
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
      // 如果是搜索状态，使用文件中的完整路径
      if (isSearching) {
        setCurrentPath(file.path);
      } else {
        const newPath = currentPath ? `${currentPath}/${file.name}` : file.name;
        setCurrentPath(newPath);
      }
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

  const handleMove = (file: FileInfo) => {
    setSelectedFile(file);
    // 获取所有文件夹，排除当前文件的路径
    const allFolders = fileManager.getAllFolders();
    const filteredFolders = allFolders.filter(folder => {
      // 排除当前文件/文件夹的路径
      const currentPath = file.path || '';
      // 排除当前文件/文件夹本身和子路径（防止循环移动）
      return !folder.startsWith(currentPath + '/') && folder !== currentPath;
    });
    setAvailableFolders(filteredFolders);
    setSelectedTargetPath(currentPath); // 默认选择当前路径
    setShowMoveDialog(true);
  };

  const handleMoveConfirm = () => {
    if (!selectedFile) {
      showNotification('请选择要移动的文件', 'warning');
      return;
    }

    const sourcePath = selectedFile.path || '';
    const targetPath = selectedTargetPath;

    // 防止移动到自己的父目录（这会导致无限递归）
    if (targetPath.startsWith(sourcePath + '/')) {
      showNotification('不能移动到子目录', 'error');
      return;
    }

    // 如果目标路径和源路径相同，无需移动
    if (targetPath === currentPath) {
      showNotification('无需移动，目标路径与当前位置相同', 'warning');
      return;
    }

    const success = fileManager.moveItem(sourcePath, targetPath);

    if (success) {
      loadFiles();
      setShowMoveDialog(false);
      setSelectedFile(null);
      setSelectedTargetPath('');
      setAvailableFolders([]);
      showNotification(`成功移动 ${selectedFile.name} 到 ${targetPath || '根目录'}`, 'success');
    } else {
      showNotification('移动失败', 'error');
    }
  };

  // Tree folder toggle functions
  const toggleFolderExpand = (folderPath: string) => {
    const newExpanded = new Set(expandedFolders);
    if (newExpanded.has(folderPath)) {
      newExpanded.delete(folderPath);
    } else {
      newExpanded.add(folderPath);
    }
    setExpandedFolders(newExpanded);
  };

  // Simple tree structure for folder display
  const renderFolderTree = () => {
    if (availableFolders.length === 0) {
      return <div className="no-folders"><span>没有可用的目标文件夹</span></div>;
    }

    // Group folders by parent path
    const folderMap = new Map<string, string[]>();
    const rootFolders: string[] = [];

    availableFolders.forEach(folder => {
      const parts = folder.split('/');
      if (parts.length === 1) {
        rootFolders.push(folder);
      } else {
        const parentPath = parts.slice(0, -1).join('/');
        if (!folderMap.has(parentPath)) {
          folderMap.set(parentPath, []);
        }
        folderMap.get(parentPath)!.push(folder);
      }
    });

    // Recursive render function
    const renderFolder = (folderPath: string, level: number = 0) => {
      const folderName = folderPath.split('/').pop() || folderPath;
      const isExpanded = expandedFolders.has(folderPath);
      const isSelected = selectedTargetPath === folderPath;
      const hasChildren = folderMap.has(folderPath);

      return (
        <div key={folderPath} style={{ marginLeft: `${level * 20}px` }}>
          <div
            className={`folder-option ${isSelected ? 'selected' : ''}`}
            onClick={() => setSelectedTargetPath(folderPath)}
          >
            <span
              className="folder-expand-icon"
              onClick={(e) => {
                e.stopPropagation();
                if (hasChildren) {
                  toggleFolderExpand(folderPath);
                }
              }}
              style={{
                cursor: hasChildren ? 'pointer' : 'default',
                width: '16px',
                display: 'inline-block',
                color: hasChildren ? '#5f6368' : 'transparent'
              }}
            >
              {hasChildren ? (isExpanded ? '▼' : '▶') : '○'}
            </span>
            <span>📁</span>
            <span>{folderName}</span>
          </div>
          {hasChildren && isExpanded && folderMap.get(folderPath)!.map(child => renderFolder(child, level + 1))}
        </div>
      );
    };

    return (
      <div>
        <div
          className={`folder-option ${selectedTargetPath === '' ? 'selected' : ''}`}
          onClick={() => setSelectedTargetPath('')}
        >
          <span className="folder-expand-icon" style={{ width: '16px', display: 'inline-block' }}>○</span>
          <span>📁</span>
          <span>根目录</span>
        </div>
        {rootFolders.map(folder => renderFolder(folder))}
      </div>
    );
  };

  const formatFileSize = (bytes: number) => {
    if (bytes === 0) return '0 B';
    const k = 1024;
    const sizes = ['B', 'KB', 'MB', 'GB', 'TB'];
    const i = Math.floor(Math.log(bytes) / Math.log(k));
    return parseFloat((bytes / Math.pow(k, i)).toFixed(2)) + ' ' + sizes[i];
  };

  const getFileExtension = (filename: string): string => {
    return filename.split('.').pop()?.toLowerCase() || '';
  };

  const isImageFile = (filename: string): boolean => {
    const imageExtensions = ['jpg', 'jpeg', 'png', 'gif', 'bmp', 'webp', 'svg', 'ico'];
    return imageExtensions.includes(getFileExtension(filename));
  };

  const isPdfFile = (filename: string): boolean => {
    return getFileExtension(filename) === 'pdf';
  };

  const isTextFile = (filename: string): boolean => {
    const textExtensions = ['txt', 'md', 'markdown', 'json', 'xml', 'csv', 'log', 'ini', 'config', 'yml', 'yaml', 'js', 'ts', 'html', 'css', 'sql', 'py', 'java', 'cpp', 'c', 'h', 'hpp', 'sh', 'bat', 'ps1'];
    return textExtensions.includes(getFileExtension(filename));
  };

  const isMarkdownFile = (filename: string): boolean => {
    const markdownExtensions = ['md', 'markdown'];
    return markdownExtensions.includes(getFileExtension(filename));
  };

  const canPreview = (file: FileInfo): boolean => {
    if (file.is_dir || !file.token) return false;
    return isImageFile(file.name) || isPdfFile(file.name) || isTextFile(file.name);
  };

  const handlePreview = (file: FileInfo) => {
    if (canPreview(file)) {
      setPreviewFile(file);
      setShowPreview(true);
    }
  };

  const closePreview = () => {
    setShowPreview(false);
    setPreviewFile(null);
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
            <div className="search-container">
              <div className="search-input-wrapper">
                <Search size={16} className="search-icon" />
                <input
                  type="text"
                  className="search-input"
                  placeholder="搜索文件或文件夹..."
                  value={searchQuery}
                  onChange={(e) => handleSearch(e.target.value)}
                />
                {searchQuery && (
                  <button
                    className="search-clear"
                    onClick={clearSearch}
                    title="清除搜索"
                  >
                    <X size={14} />
                  </button>
                )}
              </div>
            </div>
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
            <div className="file-actions-header">操作</div>
          </div>

          {files.map((file, index) => (
            <div key={index} className={`file-row ${file.is_dir ? 'folder' : 'file'}`}>
              <div
                className="file-name-cell"
                onClick={() => {
                  if (file.is_dir) {
                    handleFolderClick(file);
                  } else if (canPreview(file)) {
                    handlePreview(file);
                  }
                }}
                style={{
                  cursor: file.is_dir || canPreview(file) ? 'pointer' : 'default',
                  color: canPreview(file) ? '#1a73e8' : 'inherit'
                }}
              >
                <span className="file-icon">
                  {file.is_dir ? '📁' :
                   isImageFile(file.name) ? '🖼️' :
                   isPdfFile(file.name) ? '📕' :
                   isMarkdownFile(file.name) ? '📝' :
                   isTextFile(file.name) ? '📄' : '📄'}
                </span>
                <div className="file-info">
                  <span className="file-name">{file.name}</span>
                  {isSearching && file.path && (
                    <span className="file-path">/{file.path}</span>
                  )}
                </div>
                {file.is_dir && (
                  <span className="folder-tag">文件夹</span>
                )}
                {canPreview(file) && (
                  <span className="preview-tag">可预览</span>
                )}
              </div>
              <div className="file-size">
                {file.file_size ? formatFileSize(file.file_size) : '-'}
              </div>
              <div className="file-actions">
                <button
                  className={`btn btn-sm ${canPreview(file) ? 'btn-info' : 'btn-secondary'}`}
                  onClick={(e) => {
                    e.stopPropagation();
                    if (canPreview(file)) {
                      handlePreview(file);
                    }
                  }}
                  title={canPreview(file) ? "预览" : "不支持预览此类型"}
                  disabled={!canPreview(file)}
                >
                  <Eye size={14} />
                </button>
                <button
                  className={`btn btn-sm ${!file.is_dir ? 'btn-primary' : 'btn-secondary'}`}
                  onClick={(e) => {
                    e.stopPropagation();
                    if (!file.is_dir) {
                      handleFileDownload(file);
                    }
                  }}
                  title={!file.is_dir ? "下载" : "文件夹不支持下载"}
                  disabled={file.is_dir}
                >
                  <Download size={14} />
                </button>
                <button
                  className="btn btn-sm btn-secondary"
                  onClick={(e) => {
                    e.stopPropagation();
                    setSelectedFile(file);
                    setNewFileName(file.name);
                    setShowRenameDialog(true);
                  }}
                  title="重命名"
                >
                  重命名
                </button>
                <button
                  className="btn btn-sm btn-warning"
                  onClick={(e) => {
                    e.stopPropagation();
                    handleMove(file);
                  }}
                  title="移动"
                >
                  移动
                </button>
                <button
                  className="btn btn-sm btn-danger"
                  onClick={(e) => {
                    e.stopPropagation();
                    handleDelete(file);
                  }}
                  title="删除"
                >
                  删除
                </button>
              </div>
            </div>
          ))}

          {files.length === 0 && (
            <div className="empty-state">
              {isSearching ? (
                <p>未找到匹配 "{searchQuery}" 的文件或文件夹</p>
              ) : (
                <p>此文件夹为空</p>
              )}
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

      {/* 移动对话框 */}
      {showMoveDialog && selectedFile && (
        <div className="modal-overlay" onClick={() => setShowMoveDialog(false)}>
          <div className="modal" onClick={(e) => e.stopPropagation()}>
            <div className="modal-header">
              <h3>移动到</h3>
              <button className="modal-close" onClick={() => setShowMoveDialog(false)}>
                ×
              </button>
            </div>
            <div className="modal-body">
              <div className="form-group">
                <label>目标文件夹</label>
                <div className="folder-tree">
                  {renderFolderTree()}
                </div>
              </div>
            </div>
            <div className="modal-footer">
              <button
                className="btn btn-secondary"
                onClick={() => setShowMoveDialog(false)}
              >
                取消
              </button>
              <button
                className="btn btn-primary"
                onClick={handleMoveConfirm}
              >
                确定
              </button>
            </div>
          </div>
        </div>
      )}

      {/* 文件预览模态框 */}
      {showPreview && previewFile && (
        <FilePreview
          file={previewFile}
          onClose={closePreview}
        />
      )}
    </div>
  );
};

export default FileManager;