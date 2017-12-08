#include "MainWindow.h"
#include "AudioFile.h"
#include <Utilities/Stringify.h>

#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QResizeEvent>
#include <QFileDialog>
#include <QMessageBox>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QCheckBox>


#ifndef VERSION_NUMBER
#define VERSION_NUMBER Non-Production Build
#endif

#ifndef BUILD_NUMBER
#define BUILD_NUMBER Non-Production Build
#endif

#define MACRO_TO_STRING_INDIRECT(s) #s
#define MACRO_TO_STRING(s) MACRO_TO_STRING_INDIRECT(s)


MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
	if(this->objectName().isEmpty())
	{
		this->setObjectName(QStringLiteral("Audio Analysis Tool"));
		setWindowTitle("Audio Analysis Tool");
	}

	this->resize(startingWidth_, startingHeight_);

	SetupMenuBar();

	SetupCentralWidget();

	QMetaObject::connectSlotsByName(this);

	QObject::connect(&transientTabControl_, SIGNAL(currentChanged(int)), this, SLOT(TabChanged(int)));
	QObject::connect(transientDetectionSettings_.GetPeakThresholdLineEdit(), SIGNAL(editingFinished()), this, SLOT(PeakThresholdChanged()));
	QObject::connect(transientDetectionSettings_.GetValleyToPeakRatioLineEdit(), SIGNAL(editingFinished()), this, SLOT(ValleyToPeakRatioChanged()));
	QObject::connect(transientDetectionSettings_.GetTransientCheckBox(), SIGNAL(stateChanged(int)), this, SLOT(TransientCheckBoxChanged(int)));
}

MainWindow::~MainWindow()
{

}

void MainWindow::TabChanged(int tabNumber)
{
	waveformView_.HighlightTransient(tabNumber + 1);
}

void MainWindow::PeakThresholdChanged()
{
	// Get the value and do some basic error checking
	auto newValue{transientDetectionSettings_.GetPeakThresholdLineEdit()->text().toDouble()};
	if(newValue < .01) { newValue = 0.1;  }
	else if(newValue > 1.0) { newValue = 1.0;  }

	// Update the UI with the actual value
	transientDetectionSettings_.GetPeakThresholdLineEdit()->setText(QString::number(newValue, 'f', 2));

	// Only update and refresh the UI if an audio file is loaded and the value actually changed
	if(AudioFile().GetInstance().FileLoaded() && 
		newValue != AudioFile().GetInstance().GetTransientDetector()->GetMinimumPeakLevel())
	{
		AudioFile().GetInstance().GetTransientDetector()->Reset();
		AudioFile().GetInstance().GetTransientDetector()->SetMinimumPeakLevel(newValue);
		AudioFile().GetInstance().RefreshTransients();
		waveformView_.Update();
		transientTabControl_.Reset();
	}
}

void MainWindow::ValleyToPeakRatioChanged()
{
	// Get the value and do some basic error checking
	auto newValue{transientDetectionSettings_.GetValleyToPeakRatioLineEdit()->text().toDouble()};
	if(newValue < 1.01) { newValue = 1.01;  }
	else if(newValue > 100.0) { newValue = 100.0;  }

	// Update the UI with the actual value
	transientDetectionSettings_.GetValleyToPeakRatioLineEdit()->setText(QString::number(newValue, 'f', 2));

	// Only update and refresh the UI if an audio file is loaded and the value actually changed
	if(AudioFile().GetInstance().FileLoaded() && 
		newValue != AudioFile().GetInstance().GetTransientDetector()->GetValleyToPeakRatio())
	{
		AudioFile().GetInstance().GetTransientDetector()->Reset();
		AudioFile().GetInstance().GetTransientDetector()->SetValleyToPeakRatio(newValue);
		AudioFile().GetInstance().RefreshTransients();
		waveformView_.Update();
		transientTabControl_.Reset();
	}
}

void MainWindow::OpenFile()
{
	QString fileName = QFileDialog::getOpenFileName(this, tr("Open Wave File"), ".", tr("Wave Files (*.wav)"));
	if(!fileName.isEmpty())
	{
		std::string waveFileName{fileName.toUtf8().constData()};

		WaveFile::WaveFileReader waveFile(waveFileName);
		auto seconds{waveFile.GetSampleCount() / waveFile.GetSampleRate()};
		if(waveFile.GetChannels() != 1 || waveFile.GetBitsPerSample() != 16 || seconds < 1 || seconds > 30)
		{
			QMessageBox::about(this, tr("Audio Analysis Tool"),
				tr("<h3>Cannot load audio file</h3>"
				   "<p>Sorry, audio input files are restricted to mono, 16 bit<br>"
				   "uncompressed wave files between 1 and 30 seconds in length."));
			return;
		}

		AudioFile::GetInstance().Initialize(waveFileName);
		RefreshUIWithNewFile();
		waveformView_.Update();
		transientTabControl_.Reset();

	}
}

void MainWindow::RefreshUIWithNewFile()
{
	auto transientDetector{AudioFile::GetInstance().GetTransientDetector()};
	transientDetectionSettings_.GetPeakThresholdLineEdit()->setText(QString::number(transientDetector->GetMinimumPeakLevel(), 'f', 2));
	transientDetectionSettings_.GetValleyToPeakRatioLineEdit()->setText(QString::number(transientDetector->GetValleyToPeakRatio(), 'f', 2));
}

void MainWindow::About()
{
	// &nbsp's because I think a wider dialog box looks a little better
	std::string content{Utilities::Stringify("<h2>Audio Analysis Tool</h2>"
		"<p>A tool to... wait for it ...analyze audio. &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp;"
		"<p>Copyright &copy; 2017 Terence M. Darwen<br>"
		"More Info: <a href=http://www.tmdarwen.com>tmdarwen.com</a>"
		"<p>Version: ") + Utilities::Stringify(MACRO_TO_STRING(VERSION_NUMBER)) + Utilities::Stringify("<br>") + 
		Utilities::Stringify("Build: ") + Utilities::Stringify(MACRO_TO_STRING(BUILD_NUMBER)) + Utilities::Stringify("<br>") + 
		Utilities::Stringify("Built: ") + Utilities::Stringify(__DATE__ ) + Utilities::Stringify(" ") + Utilities::Stringify(__TIME__)};

	QMessageBox::about(this, tr("Audio Analysis Tool"), tr(content.c_str()));
}


void MainWindow::SetupMenuBar()
{
	menuBar_ = new QMenuBar(this);
	menuBar_->setObjectName(QStringLiteral("menubar"));
	menuBar_->setGeometry(QRect(0, 0, startingWidth_, 21));

	menuFile_ = new QMenu(menuBar_);
	menuFile_->setTitle("File");
	menuFile_->setObjectName(QStringLiteral("menuFile"));

	menuAbout_ = new QMenu(menuBar_);
	menuAbout_->setTitle("About");
	menuAbout_->setObjectName(QStringLiteral("menuAbout"));

	actionOpen_ = new QAction(this);
	actionOpen_->setObjectName(QStringLiteral("actionOpen_"));
	actionOpen_->setText("Open");
	connect(actionOpen_, SIGNAL(triggered()), this, SLOT(OpenFile()));

	actionAbout_ = new QAction(this);
	actionAbout_->setObjectName(QStringLiteral("actionAbout"));
	actionAbout_->setText("About");
	connect(actionAbout_, SIGNAL(triggered()), this, SLOT(About()));

	menuBar_->addAction(menuFile_->menuAction());
	menuBar_->addAction(menuAbout_->menuAction());
	menuFile_->addAction(actionOpen_);
	menuAbout_->addAction(actionAbout_);

	this->setMenuBar(menuBar_);
}

void MainWindow::SetupCentralWidget()
{
	centralWidget_ = new QWidget(this);
	centralWidget_->setObjectName(QStringLiteral("centralWidget"));
	this->setCentralWidget(centralWidget_);

	// The main display is a vertical layout with two rows.  The first (top) 
	// row contains the waveform and the second (bottom) row contains an 
	// HBoxLayout with the transient detection settings and tab control.

	auto vBoxLayout = new QVBoxLayout(centralWidget_);
	
	waveformView_.AddControl(vBoxLayout);

	auto hBoxLayout = new QHBoxLayout();

	vBoxLayout->addLayout(hBoxLayout);

	transientDetectionSettings_.AddSettings(hBoxLayout);

	transientTabControl_.AddControl(hBoxLayout);
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
	auto newSize = event->size();
	waveformView_.Resize(newSize.width(), newSize.height() / 2);
}

void MainWindow::TransientCheckBoxChanged(int state)
{
	if(state == 0)
	{
		waveformView_.DisplayTransients(false);
	}
	else
	{
		waveformView_.DisplayTransients(true);
	}
}
