#include <opencv2/dnn.hpp>

#include "Const.hpp"
class DetectObjectsModel {
   public:
    std::vector<std::string> getClassNames();
    void setClassNames(std::vector<std::string> classNames);
    std::string getModelName();
    void setModelName(std::string modelName);
    std::string getLoadedModelName();
    void setLoadedModelName(std::string loadedModelName);
    float getConfidence() const;
    void setConfidence(float confidence);

    void save(std::ofstream& out) const;
    void load(std::ifstream& in);

   private:
    float confidence = CONFIDENCE_THRESHOLD;
    std::vector<std::string> classNames;
    std::string modelName = "yolov5n";
    std::string loadedModelName = "yolov5n";
};