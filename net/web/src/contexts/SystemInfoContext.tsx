import React, { createContext, useContext, useState, useEffect, ReactNode } from 'react';
import { fileApi } from '../services/api';
import { SystemInfo } from '../types';

interface SystemInfoContextType {
  systemInfo: SystemInfo | null;
  loadSystemInfo: () => Promise<void>;
  isLoading: boolean;
}

const SystemInfoContext = createContext<SystemInfoContextType | undefined>(undefined);

export const useSystemInfo = () => {
  const context = useContext(SystemInfoContext);
  if (context === undefined) {
    throw new Error('useSystemInfo must be used within a SystemInfoProvider');
  }
  return context;
};

interface SystemInfoProviderProps {
  children: ReactNode;
}

export const SystemInfoProvider: React.FC<SystemInfoProviderProps> = ({ children }) => {
  const [systemInfo, setSystemInfo] = useState<SystemInfo | null>(null);
  const [isLoading, setIsLoading] = useState(false);

  const loadSystemInfo = async () => {
    setIsLoading(true);
    try {
      const info = await fileApi.getSystemInfo();
      setSystemInfo(info);
    } catch (error) {
      console.error('Failed to load system info:', error);
    } finally {
      setIsLoading(false);
    }
  };

  useEffect(() => {
    loadSystemInfo();
  }, []);

  const value: SystemInfoContextType = {
    systemInfo,
    loadSystemInfo,
    isLoading
  };

  return (
    <SystemInfoContext.Provider value={value}>
      {children}
    </SystemInfoContext.Provider>
  );
};