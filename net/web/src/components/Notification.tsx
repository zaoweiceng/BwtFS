import React, { useEffect, useState } from 'react';

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
    const id = Date.now().toString();
    const notificationWithId = { ...notification, id, duration: notification.duration || 3000 };

    setNotifications(prev => [...prev, notificationWithId]);

    // 自动移除通知
    setTimeout(() => {
      removeNotification(id);
    }, notificationWithId.duration);
  };

  const removeNotification = (id: string) => {
    setNotifications(prev => prev.filter(n => n.id !== id));
  };

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
          className="notification"
          style={{
            backgroundColor: getBackgroundColor(notification.type)
          }}
        >
          {notification.message}
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