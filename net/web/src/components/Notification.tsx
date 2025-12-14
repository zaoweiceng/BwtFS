import React, { useEffect, useState, useCallback } from 'react';
import { X } from 'lucide-react';

interface Notification {
  id?: string; // Make id optional for the incoming notification request
  message: string;
  type: 'success' | 'error' | 'info' | 'warning';
  duration?: number;
}

const Notification: React.FC = () => {
  const [notifications, setNotifications] = useState<Notification[]>([]);

  useEffect(() => {
    // 监听自定义事件
    const handleNotification = (event: CustomEvent<Notification>) => {
      const notification = event.detail;
      addNotification(notification);
    };

    window.addEventListener('notification', handleNotification as EventListener);

    return () => {
      window.removeEventListener('notification', handleNotification as EventListener);
    };
  }, []);

  const addNotification = (notification: Notification) => {
    // 使用更精确的ID来避免重复
    const id = `${Date.now()}-${Math.random().toString(36).substr(2, 9)}`;
    const notificationWithId = { ...notification, id, duration: notification.duration || 3000 };

    setNotifications(prev => [...prev, notificationWithId]);

    // 自动移除通知（3秒后）
    setTimeout(() => {
      removeNotification(id);
    }, notificationWithId.duration);
  };

  const removeNotification = useCallback((id: string) => {
    console.log('Attempting to remove notification:', id);
    setNotifications(prev => {
      const newNotifications = prev.filter(n => n.id !== id);
      console.log('Before removal:', prev.length, 'After removal:', newNotifications.length);
      console.log('Removed ID:', id, 'Remaining IDs:', newNotifications.map(n => n.id));
      return [...newNotifications]; // 强制创建新数组
    });
  }, []);

  const getBackgroundColor = (type: string) => {
    switch (type) {
      case 'success':
        return '#4CAF50';
      case 'error':
        return '#f44336';
      case 'warning':
        return '#ff9800';
      case 'info':
      default:
        return '#2196F3';
    }
  };

  return (
    <div className="notification-container">
      {notifications.map((notification) => (
        <div
          key={notification.id}
          className={`notification ${notification.type}`}
          style={{
            position: 'relative',
            paddingRight: '40px'
          }}
        >
          <span className="notification-message">
            {notification.message}
          </span>
          <button
            className="notification-close"
            onClick={(e) => {
              e.preventDefault();
              e.stopPropagation();
              console.log('Close button clicked!', 'Notification ID:', notification.id, 'Message:', notification.message);
              removeNotification(notification.id!);
            }}
            aria-label="关闭通知"
          >
            <X size={16} />
          </button>
        </div>
      ))}
    </div>
  );
};

// 导出触发通知的函数
export const showNotification = (message: string, type: Notification['type'] = 'info', duration?: number) => {
  const event = new CustomEvent<Notification>('notification', {
    detail: { message, type, duration }
  });
  window.dispatchEvent(event);
};

export default Notification;