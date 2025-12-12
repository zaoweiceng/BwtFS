import { FileInfo } from '../types';

const STORAGE_KEY = 'bwtfs_file_structure';

interface FileStructureItem {
  is_dir: boolean;
  token?: string;
  file_size?: number;
  children?: Record<string, FileStructureItem>;
}

type FileStructure = Record<string, FileStructureItem>;

export class FileManager {
  private fileStructure: FileStructure = {};

  constructor() {
    this.loadFromStorage();
  }

  // 从localStorage加载文件结构
  private loadFromStorage(): void {
    try {
      const stored = localStorage.getItem(STORAGE_KEY);
      if (stored) {
        this.fileStructure = JSON.parse(stored);
      }
    } catch (error) {
      console.error('Failed to load file structure from storage:', error);
      this.fileStructure = {};
    }
  }

  // 保存文件结构到localStorage
  private saveToStorage(): void {
    try {
      localStorage.setItem(STORAGE_KEY, JSON.stringify(this.fileStructure, null, 2));
    } catch (error) {
      console.error('Failed to save file structure to storage:', error);
    }
  }

  // 获取根目录结构
  getRootStructure(): FileStructure {
    return this.fileStructure;
  }

  // 列出目录内容
  listDirectory(path: string = ''): FileInfo[] {
    const files: FileInfo[] = [];
    const targetPath = path.replace(/^\/+/, '').replace(/\/+$/, '');

    let current: FileStructure = this.fileStructure;

    if (targetPath) {
      const pathParts = targetPath.split('/').filter(part => part);
      for (const part of pathParts) {
        const node = current[part];
        if (node && node.is_dir) {
          current = node.children || {};
        } else {
          return []; // 路径不存在或不是目录
        }
      }
    }

    for (const [name, node] of Object.entries(current)) {
      files.push({
        name,
        is_dir: node.is_dir,
        file_size: node.file_size || 0,
        token: node.token || '',
        path: targetPath ? `${targetPath}/${name}` : name,
        parent_path: targetPath || ''
      });
    }

    return files.sort((a, b) => {
      // 目录优先
      if (a.is_dir && !b.is_dir) return -1;
      if (!a.is_dir && b.is_dir) return 1;
      return a.name.localeCompare(b.name);
    });
  }

  // 创建目录
  createDirectory(path: string): boolean {
    const pathParts = path.replace(/^\/+/, '').replace(/\/+$/, '').split('/').filter(part => part);

    if (pathParts.length === 0) return false;

    const dirName = pathParts.pop()!;
    const parentPath = pathParts.join('/');

    let current: FileStructure = this.fileStructure;

    // 导航到目标目录
    if (parentPath) {
      for (const part of pathParts) {
        if (!current[part]) {
          return false; // 目录不存在
        }
        const node = current[part];
        if (node.is_dir) {
          current = node.children || {};
        } else {
          return false; // 路径中有文件冲突
        }
      }
    }

    // 检查目录是否已存在
    if (current[dirName]) {
      return true; // 目录已存在
    }

    // 创建新目录
    current[dirName] = {
      is_dir: true,
      children: {}
    };

    this.saveToStorage();
    return true;
  }

  // 添加文件
  addFile(path: string, token: string, fileSize: number): boolean {
    const pathParts = path.replace(/^\/+/, '').replace(/\/+$/, '').split('/').filter(part => part);

    if (pathParts.length === 0) return false;

    const fileName = pathParts.pop()!;
    const parentPath = pathParts.join('/');

    let current: FileStructure = this.fileStructure;

    // 导航到目标目录
    if (parentPath) {
      for (const part of pathParts) {
        if (!current[part]) {
          return false; // 目录不存在
        }
        const node = current[part];
        if (node.is_dir) {
          current = node.children || {};
        } else {
          return false; // 路径中有文件冲突
        }
      }
    }

    // 检查文件是否已存在
    if (current[fileName]) {
      return true; // 文件已存在
    }

    // 添加文件
    current[fileName] = {
      is_dir: false,
      token,
      file_size: fileSize
    };

    this.saveToStorage();
    return true;
  }

  // 删除文件或目录
  deleteItem(path: string): boolean {
    const pathParts = path.replace(/^\/+/, '').replace(/\/+$/, '').split('/').filter(part => part);

    if (pathParts.length === 0) return false;

    const itemName = pathParts.pop()!;
    const parentPath = pathParts.join('/');

    let current: FileStructure = this.fileStructure;

    // 导航到父目录
    if (parentPath) {
      for (const part of pathParts) {
        if (!current[part]) {
          return false;
        }
        const node = current[part];
        if (node.is_dir) {
          current = node.children || {};
        } else {
          return false;
        }
      }
    }

    // 删除项目
    if (current[itemName]) {
      delete current[itemName];
      this.saveToStorage();
      return true;
    }

    return false;
  }

  // 重命名文件或目录
  renameItem(oldPath: string, newName: string): boolean {
    const pathParts = oldPath.replace(/^\/+/, '').replace(/\/+$/, '').split('/').filter(part => part);

    if (pathParts.length === 0) return false;

    const oldName = pathParts.pop()!;
    const parentPath = pathParts.join('/');

    let current: FileStructure = this.fileStructure;

    // 导航到父目录
    if (parentPath) {
      for (const part of pathParts) {
        if (!current[part]) {
          return false;
        }
        const node = current[part];
        if (node.is_dir) {
          current = node.children || {};
        } else {
          return false;
        }
      }
    }

    // 检查旧名称是否存在，新名称是否冲突
    if (current[oldName] && !current[newName]) {
      current[newName] = current[oldName];
      delete current[oldName];
      this.saveToStorage();
      return true;
    }

    return false;
  }

  // 移动文件或目录
  moveItem(sourcePath: string, targetDir: string): boolean {
    const sourceParts = sourcePath.replace(/^\/+/, '').replace(/\/+$/, '').split('/').filter(part => part);
    const targetParts = targetDir.replace(/^\/+/, '').replace(/\/+$/, '').split('/').filter(part => part);

    if (sourceParts.length === 0) return false;

    const itemName = sourceParts.pop()!;
    const sourceParentPath = sourceParts.join('/');

    // 获取源项目
    let sourceCurrent: FileStructure = this.fileStructure;
    if (sourceParentPath) {
      for (const part of sourceParts) {
        if (!sourceCurrent[part]) return false;
        const node = sourceCurrent[part];
        if (node.is_dir) {
          sourceCurrent = node.children || {};
        } else {
          return false;
        }
      }
    }

    const sourceItem = sourceCurrent[itemName];
    if (!sourceItem) return false;

    // 检查目标目录是否存在
    let targetCurrent: FileStructure = this.fileStructure;
    if (targetDir) {
      for (const part of targetParts) {
        if (!targetCurrent[part]) return false;
        const node = targetCurrent[part];
        if (node.is_dir) {
          targetCurrent = node.children || {};
        } else {
          return false;
        }
      }
    }

    // 检查目标目录中是否有同名文件
    if (targetCurrent[itemName]) {
      return false;
    }

    // 移动项目
    targetCurrent[itemName] = sourceItem;
    delete sourceCurrent[itemName];
    this.saveToStorage();
    return true;
  }

  // 导出文件结构为JSON
  exportStructure(): string {
    return JSON.stringify(this.fileStructure, null, 2);
  }

  // 从JSON导入文件结构
  importStructure(jsonString: string): boolean {
    try {
      const structure = JSON.parse(jsonString) as FileStructure;
      this.fileStructure = structure;
      this.saveToStorage();
      return true;
    } catch (error) {
      console.error('Failed to import file structure:', error);
      return false;
    }
  }

  // 获取所有文件夹列表（用于移动操作）
  getAllFolders(): string[] {
    const folders: string[] = [];

    // 递归获取所有文件夹
    const collectFolders = (current: FileStructure, currentPath: string = ''): void => {
      for (const [name, node] of Object.entries(current)) {
        if (node.is_dir) {
          const fullPath = currentPath ? `${currentPath}/${name}` : name;
          folders.push(fullPath);

          if (node.children) {
            collectFolders(node.children, fullPath);
          }
        }
      }
    };

    collectFolders(this.fileStructure);
    return folders.sort((a, b) => a.localeCompare(b));
  }

  // 递归搜索文件和文件夹
  searchFiles(query: string, searchPath: string = ''): FileInfo[] {
    const results: FileInfo[] = [];
    const queryLower = query.toLowerCase();

    if (!query.trim()) {
      return results;
    }

    // 递归搜索函数
    const searchRecursive = (current: FileStructure, currentPath: string, depth: number = 0): void => {
      // 防止无限递归
      if (depth > 10) return;

      for (const [name, node] of Object.entries(current)) {
        const fullPath = currentPath ? `${currentPath}/${name}` : name;

        // 检查名称是否匹配查询
        if (name.toLowerCase().includes(queryLower)) {
          results.push({
            name,
            is_dir: node.is_dir,
            file_size: node.file_size || 0,
            token: node.token || '',
            path: fullPath,
            parent_path: currentPath
          });
        }

        // 如果是目录，递归搜索子目录
        if (node.is_dir && node.children) {
          searchRecursive(node.children, fullPath, depth + 1);
        }
      }
    };

    // 如果指定了搜索路径，从该路径开始搜索；否则从根目录搜索
    if (searchPath) {
      const targetPath = searchPath.replace(/^\/+/, '').replace(/\/+$/, '');
      let current: FileStructure = this.fileStructure;

      if (targetPath) {
        const pathParts = targetPath.split('/').filter(part => part);
        for (const part of pathParts) {
          const node = current[part];
          if (node && node.is_dir) {
            current = node.children || {};
          } else {
            return []; // 路径不存在或不是目录
          }
        }
      }

      searchRecursive(current, targetPath);
    } else {
      searchRecursive(this.fileStructure, '');
    }

    // 按类型和名称排序：目录优先，然后按名称排序
    return results.sort((a, b) => {
      // 目录优先
      if (a.is_dir && !b.is_dir) return -1;
      if (!a.is_dir && b.is_dir) return 1;
      // 按名称排序
      return a.name.localeCompare(b.name);
    });
  }

  // 获取文件信息
  getFileInfo(path: string): FileInfo | null {
    const pathParts = path.replace(/^\/+/, '').replace(/\/+$/, '').split('/').filter(part => part);

    if (pathParts.length === 0) return null;

    const fileName = pathParts.pop()!;
    const parentPath = pathParts.join('/');

    let current: FileStructure = this.fileStructure;

    // 导航到父目录
    if (parentPath) {
      for (const part of pathParts) {
        if (!current[part]) {
          return null;
        }
        const node = current[part];
        if (node.is_dir) {
          current = node.children || {};
        } else {
          return null;
        }
      }
    }

    const item = current[fileName];
    if (item) {
      return {
        name: fileName,
        is_dir: item.is_dir,
        file_size: item.file_size || 0,
        token: item.token || '',
        path,
        parent_path: parentPath
      };
    }

    return null;
  }
}

// 创建全局实例
export const fileManager = new FileManager();