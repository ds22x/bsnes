struct Utility {
  void setMode(Interface::Mode mode);
  void resizeMainWindow(bool shrink = false);
  void shrinkMainWindow();
  void toggleFullScreen();
};

extern Utility *utility;