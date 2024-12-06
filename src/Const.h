#pragma once

#include <string>
#include <QString>


const std::string IMAGESDATA_SAVE_DAT_PATH = "/home/eugene/save.dat";
const std::string DELETED_IMAGESDATA_SAVE_DAT_PATH = "/home/eugene/deleted.dat";

const std::string THUMBNAIL_PATH = "/home/eugene/.cache/thumbnails";

const QString IMAGE_PATH_LOADING = ":/loading.png";


// const std::string RESSOURCE = "../src/ressources/";


const int PREVIEW_NBR = 3;
const int PRE_LOAD_RADIUS = 5; // on en precharge 2 de plus comme ça l'utilisateut ne vois meme pas les images charger
const int TIME_BEFORE_FULL_QUALITY = 200;
const int TIME_BEFORE_PRE_LOAD_FULL_QUALITY = 300;

const int LINE_LOADED = 2; // fois ou l'on charge un tableau d'image (2 = 2 tableau pres charger)
const int TIME_BEFORE_LOAD = 100;



const QString CLICK_BACKGROUND_COLOR = "#9c9c9c";
const QString HOVER_BACKGROUND_COLOR = "#b3b3b3";
const QString BACKGROUND_COLOR = "transparent";



const QString IMAGE_EDITOR_WINDOW_NAME = "EasyImageEditor : Image Editor Window";
const QString IMAGE_BOOTH_WINDOW_NAME = "EasyImageEditor : Image Booth Window";


