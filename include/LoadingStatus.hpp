#pragma once
#include <fstream>

class LoadingStatus {
   private:
    enum class LoadingStatusType {
        NotLoaded,
        Loading,
        Loaded
    };

   public:
    void setStatus(LoadingStatusType status);
    void setStatusLoading();
    void setStatusNotLoaded();
    void setStatusLoaded();
    bool isStatusLoading();
    bool isStatusNotLoaded();
    bool isStatusLoaded();
    void save(std::ofstream& out) const;
    void load(std::ifstream& in);

   private:
    LoadingStatusType status = LoadingStatusType::NotLoaded;
};