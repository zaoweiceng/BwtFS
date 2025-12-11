export interface FileInfo {
  name: string;
  is_dir: boolean;
  file_size?: number;
  token?: string;
  path: string;
  parent_path?: string;
}

export interface SystemInfo {
  file_size: number;
  block_size: number;
  block_count: number;
  used_size: number;
  total_size: number;
  free_size: number;
  create_time: number;
  modify_time: number;
}

export interface UploadProgress {
  loaded: number;
  total: number;
  percentage: number;
}

export interface FileTreeNode {
  [key: string]: FileTreeNode | FileInfo;
}

export interface ApiResponse<T = any> {
  status: string;
  message?: string;
  data?: T;
  token?: string;
  chunk?: number;
}

export interface FileStructure {
  [key: string]: FileStructure | FileInfo;
}