#pragma once
#include <vector>
#include <unordered_map>
#include <glm/glm.hpp>
#include "Bullet.h"

// 空间网格加速结构
class SpatialGrid {
private:
    struct Cell {
        std::vector<Bullet*> bullets;
    };
    
    std::unordered_map<int, Cell> mGrid;
    float mCellSize;
    int mGridWidth, mGridHeight;
    
    // 计算网格坐标
    int getCellIndex(const glm::vec2& pos) const {
        int x = static_cast<int>(pos.x / mCellSize);
        int y = static_cast<int>(pos.y / mCellSize);
        return y * mGridWidth + x;
    }
    
public:
    SpatialGrid(float cellSize = 64.0f, int gridW = 14, int gridH = 16)
        : mCellSize(cellSize), mGridWidth(gridW), mGridHeight(gridH) {}
    
    void clear() {
        for (auto& [key, cell] : mGrid) {
            cell.bullets.clear();
        }
    }
    
    // 插入子弹
    void insert(Bullet* bullet) {
        int idx = getCellIndex(bullet->getPosition());
        mGrid[idx].bullets.push_back(bullet);
    }
    
    // 查询附近的子弹
    std::vector<Bullet*> query(const glm::vec2& pos, float radius) {
        std::vector<Bullet*> results;
        
        // 计算需要检查的网格范围
        int minX = static_cast<int>((pos.x - radius) / mCellSize);
        int maxX = static_cast<int>((pos.x + radius) / mCellSize);
        int minY = static_cast<int>((pos.y - radius) / mCellSize);
        int maxY = static_cast<int>((pos.y + radius) / mCellSize);
        
        for (int y = minY; y <= maxY; ++y) {
            for (int x = minX; x <= maxX; ++x) {
                int idx = y * mGridWidth + x;
                auto it = mGrid.find(idx);
                if (it != mGrid.end()) {
                    results.insert(results.end(), 
                                   it->second.bullets.begin(), 
                                   it->second.bullets.end());
                }
            }
        }
        
        return results;
    }
};