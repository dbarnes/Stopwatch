#pragma once

#include <iostream>
#include <map>
#include <unordered_set>
#include <utility>

#include <QApplication>
#include <QCheckBox>
#include <QKeyEvent>
#include <QTableWidget>
#include <QUdpSocket>

#include "RingBuffer.h"
#include "StopwatchDecoder.h"
#include "plotHolderWidget.h"

class StopwatchViewer : public QWidget {
  Q_OBJECT

 public:
  StopwatchViewer(QWidget* parent = 0);
  ~StopwatchViewer();

 private:
  static const int NUM_FIELDS = 7;

  QUdpSocket* udpSocket = nullptr;

  bool plotAverages;

  class TableRow {
   public:
    TableRow() {}

    virtual ~TableRow() {
      delete[] tableItems;
      delete checkItem;
    }

    inline bool isUninit() const {
      return tableItems == 0 || checkItem == 0;
    }

    int row = 0;
    QTableWidgetItem* tableItems = nullptr;
    QCheckBox* checkItem = nullptr;
  };

  PlotHolderWidget* plotHolderWidget = nullptr;
  QTableWidget* tableWidget = nullptr;
  QComboBox* plotChoice = nullptr;

  std::map<
      uint64_t,
      std::map<std::string, std::pair<RingBuffer<float, DEFAULT_RINGBUFFER_SIZE>, TableRow>>>
      cache;

  int lastRow = 0;

  void updateTable();
  void keyPressEvent(QKeyEvent* event) override;

  std::unordered_set<std::string> enabledBeforeReset;

 private slots:
  void processPendingDatagram();
  void flushCache();
  void checkboxHit();
};
