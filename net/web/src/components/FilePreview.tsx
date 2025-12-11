import React, { useState, useEffect } from 'react';
import ReactMarkdown from 'react-markdown';
import remarkGfm from 'remark-gfm';
import { X, Download, Copy, Check } from 'lucide-react';
import { fileApi } from '../services/api';
import { showNotification } from './Notification';
import { FileInfo } from '../types';

interface FilePreviewProps {
  file: FileInfo;
  onClose: () => void;
}

const FilePreview: React.FC<FilePreviewProps> = ({ file, onClose }) => {
  const [content, setContent] = useState<string>('');
  const [imageUrl, setImageUrl] = useState<string>('');
  const [pdfUrl, setPdfUrl] = useState<string>('');
    const [loading, setLoading] = useState(true);
  const [copied, setCopied] = useState(false);
  const [error, setError] = useState<string>('');

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

  
  const formatFileSize = (bytes: number): string => {
    if (bytes === 0) return '0 B';
    const k = 1024;
    const sizes = ['B', 'KB', 'MB', 'GB'];
    const i = Math.floor(Math.log(bytes) / Math.log(k));
    return parseFloat((bytes / Math.pow(k, i)).toFixed(2)) + ' ' + sizes[i];
  };

  useEffect(() => {
    loadFileContent();

    // 清理函数，防止内存泄漏
    return () => {
      if (imageUrl) {
        URL.revokeObjectURL(imageUrl);
      }
      if (pdfUrl) {
        URL.revokeObjectURL(pdfUrl);
      }
    };
  }, [file]);

  const loadFileContent = async () => {
    console.log('Loading file content for:', file.name, 'Type:', getFileExtension(file.name));

    if (!file.token) {
      setError('文件Token不存在');
      setLoading(false);
      return;
    }

    try {
      setLoading(true);
      setError('');

      if (isImageFile(file.name)) {
        // 对于图片文件，直接创建URL
        const blob = await fileApi.downloadFile(file.token);
        const url = URL.createObjectURL(blob);
        setImageUrl(url);
        setContent('');
        setPdfUrl('');
      } else if (isPdfFile(file.name)) {
        // 对于PDF文件，先检查缓存，没有则下载到缓存
        try {
          console.log('Loading PDF for token:', file.token);

          // 检查缓存中是否已有该PDF
          const cacheKey = `pdf_${file.token}`;
          let pdfBlob: Blob;

          // 尝试从IndexedDB缓存获取
          const cacheName = 'bwtfs-pdf-cache';
          const cache = await caches.open(cacheName);
          const cachedResponse = await cache.match(cacheKey);

          if (cachedResponse) {
            console.log('PDF found in cache');
            pdfBlob = await cachedResponse.blob();
          } else {
            console.log('PDF not in cache, downloading from server');
            // 使用fileApi下载文件
            pdfBlob = await fileApi.downloadFile(file.token);

            // 存储到缓存
            await cache.put(cacheKey, new Response(pdfBlob, {
              headers: { 'Content-Type': 'application/pdf' }
            }));
            console.log('PDF cached successfully');
          }

          console.log('PDF blob size:', pdfBlob.size, 'type:', pdfBlob.type);

          // 确保blob类型为application/pdf
          const pdfBlobWithType = new Blob([pdfBlob], { type: 'application/pdf' });
          console.log('Corrected PDF blob type:', pdfBlobWithType.type);

          // 直接使用blob URL，不转换为data URL（避免长度限制）
          const blobUrl = URL.createObjectURL(pdfBlobWithType);
          console.log('PDF blob URL created:', blobUrl);
          setPdfUrl(blobUrl);
          setImageUrl('');
          setContent('');
          setLoading(false);

        } catch (error) {
          console.error('Failed to load PDF:', error);
          setError('PDF加载失败: ' + (error as Error).message);
          setLoading(false);
        }
      } else if (isTextFile(file.name)) {
        // 对于文本文件，读取内容
        const blob = await fileApi.downloadFile(file.token);
        const text = await blob.text();
        setContent(text);
        setImageUrl('');
        setPdfUrl('');
      } else {
        setError('不支持的文件类型预览');
      }
    } catch (error) {
      console.error('Failed to load file content:', error);
      setError('加载文件内容失败');
    } finally {
      setLoading(false);
    }
  };

  const handleCopyContent = async () => {
    if (content) {
      try {
        await navigator.clipboard.writeText(content);
        setCopied(true);
        showNotification('内容已复制到剪贴板', 'success');
        setTimeout(() => setCopied(false), 2000);
      } catch (error) {
        showNotification('复制失败', 'error');
      }
    }
  };

  const handleDownload = () => {
    if (file.token) {
      const link = document.createElement('a');
      link.href = `/${file.token}`;
      link.download = file.name;
      document.body.appendChild(link);
      link.click();
      document.body.removeChild(link);
    }
  };

  const renderContent = () => {
    console.log('renderContent called - loading:', loading, 'error:', error, 'pdfUrl:', !!pdfUrl, 'imageUrl:', !!imageUrl, 'content:', !!content);

    if (loading) {
      return (
        <div className="preview-loading">
          <div className="loading-spinner"></div>
          <p>正在加载文件内容...</p>
        </div>
      );
    }

    if (error) {
      return (
        <div className="preview-error">
          <p>❌ {error}</p>
        </div>
      );
    }

    if (imageUrl) {
      return (
        <div className="preview-image">
          <img
            src={imageUrl}
            alt={file.name}
            style={{
              maxWidth: '100%',
              maxHeight: '70vh',
              objectFit: 'contain'
            }}
          />
        </div>
      );
    }

    if (pdfUrl) {
      return (
        <div className="preview-pdf">
          <div className="pdf-info">
            <p>PDF文件预览</p>
          </div>
          <div className="pdf-container">
            {loading && (
              <div className="preview-loading">
                <div className="loading-spinner"></div>
                <p>正在加载PDF...</p>
              </div>
            )}
            <iframe
              src={pdfUrl}
              style={{
                width: '100%',
                height: '70vh',
                border: '1px solid #e0e0e0',
                borderRadius: '8px',
                backgroundColor: 'white',
                display: loading ? 'none' : 'block'
              }}
              title="PDF预览"
              onLoad={() => {
                console.log('PDF iframe loaded successfully');
                setLoading(false);
              }}
              onError={() => {
                console.error('PDF iframe failed to load');
                setError('PDF加载失败，请尝试下载文件查看');
                setLoading(false);
              }}
            />
          </div>
        </div>
      );
    }

    if (content) {
      if (isMarkdownFile(file.name)) {
        return (
          <div className="preview-markdown">
            <div className="markdown-content">
              <ReactMarkdown
                remarkPlugins={[remarkGfm]}
              >
                {content}
              </ReactMarkdown>
            </div>
          </div>
        );
      } else {
        return (
          <div className="preview-text">
            <pre className="text-content">
              <code>{content}</code>
            </pre>
          </div>
        );
      }
    }

    return (
      <div className="preview-error">
        <p>无法预览此文件类型</p>
      </div>
    );
  };

  return (
    <div className="modal-overlay" onClick={onClose}>
      <div className="file-preview-modal" onClick={(e) => e.stopPropagation()}>
        <div className="preview-header">
          <div className="preview-info">
            <h3 className="preview-title">{file.name}</h3>
            {file.file_size && (
              <span className="preview-size">{formatFileSize(file.file_size)}</span>
            )}
          </div>
          <div className="preview-actions">
            {content && (
              <button
                className="btn btn-sm btn-secondary"
                onClick={handleCopyContent}
                title={copied ? "已复制" : "复制内容"}
              >
                {copied ? <Check size={14} /> : <Copy size={14} />}
                {copied ? '已复制' : '复制'}
              </button>
            )}
            <button
              className="btn btn-sm btn-primary"
              onClick={handleDownload}
              title="下载文件"
            >
              <Download size={14} />
              下载
            </button>
            <button
              className="btn btn-sm btn-secondary"
              onClick={onClose}
              title="关闭"
            >
              <X size={14} />
            </button>
          </div>
        </div>
        <div className="preview-content">
          {renderContent()}
        </div>
      </div>
    </div>
  );
};

export default FilePreview;