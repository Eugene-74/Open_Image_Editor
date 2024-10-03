function Controller() {
    this.silent = true; // run in silent mode
}

Controller.prototype.Illuminate = function() {
    installer.setProductName("Qt");
    installer.setApplicationName("Qt Installer");
    installer.setVersion("6.7.0");

    // Select the components to install
    installer.addDefaultInstall();
    installer.addDefaultComponent("qt.tools.qtcreator");  // Add Qt Creator
    installer.addDefaultComponent("qt.6.7.0.qtbase");     // Qt Base
    installer.addDefaultComponent("qt.6.7.0.qtdeclarative"); // Qt Declarative
    installer.addDefaultComponent("qt.6.7.0.qtmultimedia");  // Qt Multimedia
    installer.addDefaultComponent("qt.6.7.0.qtopengl");      // Qt OpenGL
    installer.addDefaultComponent("qt.6.7.0.qtnetwork");     // Qt Network
    installer.addDefaultComponent("qt.6.7.0.qtsvg");         // Qt SVG
    installer.addDefaultComponent("qt.6.7.0.qtwidgets");     // Qt Widgets

    // Set the install location
    installer.setInstallLocation("C:/Qt");
};

Controller.prototype.ComponentSelectionChanged = function() {
    // Optional: handle component selection changes
};

Controller.prototype.licenseAgreement = function() {
    return true; // accept license agreement
};
