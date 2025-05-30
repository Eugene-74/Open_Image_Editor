#include <opencv2/dnn.hpp>

#include "Const.hpp"
class DetectObjectsModel {
   public:
    std::vector<std::string> getClassNames();
    void setClassNames(std::vector<std::string> classNames);
    std::string getModelName();
    void setModelName(std::string modelName);
    float getConfidence() const;
    void setConfidence(float confidence);

    void save(std::ofstream& out) const;
    void load(std::ifstream& in);

   private:
    float confidence = Const::Yolo::CONFIDENCE_THRESHOLD;
    std::vector<std::string> classNames;
    std::string modelName = Const::Model::YoloV5::Names::N;
};