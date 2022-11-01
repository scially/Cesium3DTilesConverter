#pragma once

#include <OSGBConvert.h>
#include <spdlog/spdlog.h>

#include <QObject>
#include <QRunnable>
#include <QString>
#include <QThread>

namespace scially {
    class OSGBConverter: public QObject{
        Q_OBJECT
    public:
        OSGBConverter(QObject *parent = nullptr): QObject(parent) {}

        void setInput(const QString& input) noexcept { input_ = input; }
        void setOutput(const QString& output) noexcept { output_ = output; }
        void setHeight(double height) noexcept { height_ = height; }
        void setMaxLevel(int maxLevel) noexcept { maxLevel_ = maxLevel; }
        void setThreadCount(int threadCount) noexcept { 
           threadCount_ = threadCount == 0 ? QThread::idealThreadCount() : threadCount;
        }
        void run();

        QString input_;
        QString output_;
        double height_ = 0;
        int threadCount_ = 0;
        int maxLevel_ = std::numeric_limits<int>::max();
    };

    class OSGBTileConverterTask: public QObject, public QRunnable{
        Q_OBJECT
    public:
        OSGBTileConverterTask(QObject* parent = nullptr): QObject(parent) { setAutoDelete(false); }
        
        void setTileLocation(const QString& input) noexcept { osgbLevel_.setTileLocation(input); }
        void setOutput(const QString& output) noexcept { output_ = output; }
        void setMaxLevel(int maxLevel) noexcept{ maxLevel_ = maxLevel; }
        int maxLevel() const noexcept { return maxLevel_; }
        QString output() const noexcept { return output_; }
        bool isSuccess() const noexcept { return success_; }
        const BaseTile& baseTile() const noexcept { return tile_; }
        const OSGBLevel& osgbLevel() const noexcept { return osgbLevel_; }

        virtual void run() override {
            spdlog::info("start process tile: {}", osgbLevel_.nodeName);
            try{
                success_ = osgbLevel_.convertTiles(tile_, output_, maxLevel_);
            }catch(...){
                spdlog::error("Unkown error");
                return;
            }

            if(success_){
                spdlog::info("finish process tile {} success", osgbLevel_.nodeName) ;
            }else{
                spdlog::error("finish process tile {} failed ", osgbLevel_.nodeName) ;
            }
        }

        OSGBLevel osgbLevel_;
        bool success_;
        BaseTile tile_;
        QString output_;
        int maxLevel_;
    };
}
