#pragma once

#include <QStandardPaths>
#include <QString>
#include <map>
#include <string>

#include "Box.hpp"

const std::string APP_NAME = "OpenImageEditor";

const float INPUT_WIDTH = 640.0;
const float INPUT_HEIGHT = 640.0;
const float SCORE_THRESHOLD = 0.2;
const float CONFIDENCE_THRESHOLD = 0.4;

const int IMAGE_PER_THREAD = 20;

const QString DESCKTOP_PATH = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
const QString DOCUMENTS_PATH = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
const QString PICTURES_PATH = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
const QString HOME_PATH = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
const QString CACHE_PATH = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
const QString APPDATA_PATH = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
const QString APP_FILES = APPDATA_PATH + "/" + QString::fromStdString(APP_NAME);

const QString DOWNLOAD_PATH = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);

const std::vector<std::string> filesToDownload = {"coco.names", "yolov5n.onnx", "yolov5s.onnx", "yolov5m.onnx", "yolov5l.onnx", "yolov5x.onnx"};

const int PREVIEW_NBR = 3;
const int PRE_LOAD_RADIUS = 5;  // on en precharge 2 de plus comme ça l'utilisateut ne vois meme pas les images charger
const int TIME_BEFORE_FULL_QUALITY = 200;

const int IMAGE_BOOTH_PRE_LOAD_RADIUS = 5;  // x pages d'image

const int FREE_THREAD = 2;
const int PRE_LOAD_RADIUS_IMAGE_BOOTH = 5;

const int TIME_UNDO_VISUALISATION = 500;

const int TIME_BEFORE_REZISE = 100;

const std::string CLICK_BACKGROUND_COLOR = "#9c9c9c";
const std::string HOVER_BACKGROUND_COLOR = "#b3b3b3";
const std::string BACKGROUND_COLOR = "transparent";

const QString IMAGE_EDITOR_WINDOW_NAME = QString::fromStdString(APP_NAME) + " : Image Editor Window";
const QString IMAGE_BOOTH_WINDOW_NAME = QString::fromStdString(APP_NAME) + " : Image Booth Window";
const QString MAIN_WINDOW_WINDOW_NAME = QString::fromStdString(APP_NAME) + " : Image Booth Window";

const int IMAGE_BOOTH_IMAGE_QUALITY = 256;
const int IMAGE_BOOTH_IMAGE_POOR_QUALITY = 16;

#ifdef _WIN32
const std::string SAVE_PATH = APPDATA_PATH.toStdString() + "/" + APP_NAME;
#else
const std::string SAVE_PATH = HOME_PATH.toStdString() + "/" + APP_NAME;
#endif

const std::string THUMBNAIL_PATH = SAVE_PATH + "/thumbnails";

const std::string IMAGESDATA_SAVE_DATA_PATH = SAVE_PATH + "/save." + APP_NAME;

const std::map<std::string, Option> DEFAULT_OPTIONS = {
    {"Sizes::imageBooth::ImagesPerLine", Option("text", "10")}};

const std::string COLOR_BACKGROUND_IMAGE_BOOTH_SELECTED = "red";
const std::string COLOR_BACKGROUND_HOVER_IMAGE_BOOTH_SELECTED = "red";

const std::string COLOR_BACKGROUND_IMAGE_BOOTH_SELECTED_MULTIPLE_SELECT = "blue";
const std::string COLOR_BACKGROUND_HOVER_IMAGE_BOOTH_SELECTED_MULTIPLE_SELECT = "blue";

const std::string COLOR_BACKGROUND_IMAGE_BOOTH_SELECTED_MULTIPLE_UNSELECT = "green";
const std::string COLOR_BACKGROUND_HOVER_IMAGE_BOOTH_SELECTED_MULTIPLE_UNSELECT = "green";

class Const {
   public:
    class Orientation {
       public:
        static const int UNDEFINED = 0;
        static const int NORMAL = 1;
        static const int FLIP_HORIZONTAL = 2;
        static const int ROTATE_180 = 3;
        static const int FLIP_VERTICAL = 4;
        static const int TRANSPOSE = 5;
        static const int ROTATE_90 = 6;
        static const int TRANSVERSE = 7;
        static const int ROTATE_270 = 8;
    };
    class Rotation {
       public:
        static const int LEFT = -90;
        static const int RIGHT = 90;
        static const int UP_SIDE_DOWN = 180;
    };

    class Thread {
       public:
        static const int MAX_IN_QUEUE = 100000;
    };
    class Thumbnail {
       public:
        static constexpr int POOR_QUALITY = 16;
        static constexpr int NORMAL_QUALITY = 128;
        static constexpr int GOOD_QUALITY = 256;
        static constexpr int HIGHT_QUALITY = 512;
        static constexpr std::array<int, 4> THUMBNAIL_SIZES = {16, 128, 256, 512};
    };
    class Yolo {
       public:
        static constexpr float INPUT_WIDTH = 640;
        static constexpr float INPUT_HEIGHT = 640;
        static constexpr float SCORE_THRESHOLD = 0.2;
        static constexpr float CONFIDENCE_THRESHOLD = 0.4;
    };
    class ImageBooth {
       public:
        static const int IMAGE_QUALITY = Const::Thumbnail::GOOD_QUALITY;
        static const int IMAGE_POOR_QUALITY = Const::Thumbnail::POOR_QUALITY;
    };
    class ImageEditor {
       public:
        static const int IMAGE_QUALITY = Const::Thumbnail::HIGHT_QUALITY;
        static const int IMAGE_POOR_QUALITY = Const::Thumbnail::POOR_QUALITY;
        static const int MAX_NAME_LENGTH = 50;
    };
    class Extension {
       public:
        static const std::vector<std::string> IMAGE;
        static const std::vector<std::string> VIDEO;
        static const std::vector<std::string> IMAGE_CONVERTION;

        static const std::vector<std::string> TURNABLE_IMAGE;
        static const std::vector<std::string> MIRRORABLE_IMAGE;
        static const std::vector<std::string> METADA_ACTION_IMAGE;
        static const std::vector<std::string> REAL_ACTION_IMAGE;
    };

    class ImagePath {
       public:
        static const QString LOADING;
        static const QString ERROR_PATH;  // can't use ERROR because windows use it
    };
    class IconPath {
       public:
        static const QString DELETE_ICON;  // can't use DELETE because windows use it
        static const QString SAVE;
        static const QString EXPORT;
        static const QString ROTATE_RIGHT;
        static const QString ROTATE_LEFT;
        static const QString MIRROR_UP_DOWN;
        static const QString MIRROR_LEFT_RIGHT;
        static const QString CONVERSION;
        static const QString EDIT_EXIF;
        static const QString EDIT_PERSONS;
        static const QString EDIT_FILTERS;

        static const QString FOLDER;
        static const QString ALL_IMAGES;
        static const QString BACK;

        static const QString BEFORE;
        static const QString NEXT;

        static const QString IMAGE_BOOTH;
        static const QString ADD_IMAGES;

        static const QString DISCORD;
        static const QString GITHUB;
        static const QString OPTION;
    };
    class Tooltip {
       public:
        static const QString DISCORD;
        static const QString GITHUB;
        static const QString PARAMETER;

        static const QString OPEN_IMAGE_BOOTH;
        static const QString OPEN_IMAGE_EDITOR;
        static const QString ADD_IMAGES;

        static const QString BACK;

        class ImageBooth {
           public:
            static const QString DELETE_TIP;  // can't use DELETE because windows use it
            static const QString SAVE;
            static const QString EXPORT;
            static const QString ROTATE_RIGHT;
            static const QString ROTATE_LEFT;
            static const QString MIRROR_UP_DOWN;
            static const QString MIRROR_LEFT_RIGHT;
            static const QString CONVERSION;

            static const QString EDIT_EXIF;
            static const QString EDIT_PERSONS;
            static const QString EDIT_FILTERS;

            static const QString ALL_IMAGES;
        };
        class ImageEditor {
           public:
            static const QString DELETE_TIP;  // can't use DELETE because windows use it
            static const QString SAVE;
            static const QString EXPORT;
            static const QString ROTATE_RIGHT;
            static const QString ROTATE_LEFT;
            static const QString MIRROR_UP_DOWN;
            static const QString MIRROR_LEFT_RIGHT;
            static const QString EDIT_EXIF;
            static const QString EDIT_PERSONS;
            static const QString CONVERSION;
        };
    };
};