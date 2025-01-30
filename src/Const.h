#pragma once

#include <string>
#include <QString>
#include <QStandardPaths>
#include "display/box/Box.h"

const std::string APP_NAME = "OpenImageEditor";

const QString DESCKTOP_PATH = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
const QString DOCUMENTS_PATH = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
const QString PICTURES_PATH = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
const QString HOME_PATH = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
const QString CACHE_PATH = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);


const QString IMAGE_PATH_LOADING = ":/loading.png";

const int PREVIEW_NBR = 3;
const int PRE_LOAD_RADIUS = 5; // on en precharge 2 de plus comme ça l'utilisateut ne vois meme pas les images charger
const int TIME_BEFORE_FULL_QUALITY = 200;

const int LINE_LOADED = 2; // fois ou l'on charge un tableau d'image (2 = 2 tableau pres charger)


const QString CLICK_BACKGROUND_COLOR = "#9c9c9c";
const QString HOVER_BACKGROUND_COLOR = "#b3b3b3";
const QString BACKGROUND_COLOR = "transparent";



const QString IMAGE_EDITOR_WINDOW_NAME = QString::fromStdString(APP_NAME) + " : Image Editor Window";
const QString IMAGE_BOOTH_WINDOW_NAME = QString::fromStdString(APP_NAME) + " : Image Booth Window";
const QString MAIN_WINDOW_WINDOW_NAME = QString::fromStdString(APP_NAME) + " : Image Booth Window";

const int IMAGE_BOOTH_IMAGE_QUALITY = 256; //128 or 256 or 512


const std::string THUMBNAIL_PATH_OPTION = "thumbnail save path";

const std::string SAVE_PATH = HOME_PATH.toStdString() + "/." + APP_NAME;
const std::string IMAGESDATA_SAVE_DATA_PATH = SAVE_PATH + "/save." + APP_NAME;



const std::map<std::string, Option> DEFAULT_OPTIONS = {
    {THUMBNAIL_PATH_OPTION, Option("directory", SAVE_PATH + "/thumbnails")},
};


const std::vector<std::string> IMAGE_EXTENSIONS = { ".jpg", ".jpeg", ".png", ".heic",".webp",".svg" ,".tiff",".bmp",".gif" };
const std::vector<std::string> TURNABLE_IMAGE_EXTENSIONS = { ".jpg", ".jpeg",".png",".heic",".webp",".tiff",".bmp" };
const std::vector<std::string> MIRORABLE_IMAGE_EXTENSIONS = { ".jpg", ".jpeg",".png",".webp",".heic",".tiff",".bmp" };
const std::vector<std::string> METADA_ACTION_IMAGE_EXTENSIONS = { ".jpg", ".jpeg",  ".heic",".webp",".svg" ,".tiff" };
const std::vector<std::string> REAL_ACTION_IMAGE_EXTENSIONS = { ".png",".bmp" };

const std::string COLOR_BACKGROUND_IMAGE_BOOTH_SELECTED = "red";
const std::string COLOR_BACKGROUND_HOVER_IMAGE_BOOTH_SELECTED = "red";

const std::string COLOR_BACKGROUND_IMAGE_BOOTH_SELECTED_MULTIPLE_SELECT = "blue";
const std::string COLOR_BACKGROUND_HOVER_IMAGE_BOOTH_SELECTED_MULTIPLE_SELECT = "blue";

const std::string COLOR_BACKGROUND_IMAGE_BOOTH_SELECTED_MULTIPLE_UNSELECT = "green";
const std::string COLOR_BACKGROUND_HOVER_IMAGE_BOOTH_SELECTED_MULTIPLE_UNSELECT = "green";