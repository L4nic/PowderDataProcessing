// powder_processing.cpp
#include <QApplication>
#include <QMainWindow>
#include <QTabWidget>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QFileDialog>
#include <QMessageBox>
#include <QString>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <cmath>
#include <stdexcept>
#include <QTextEdit>


// Fonction utilitaire (similaire à mid$ en Liberty BASIC)
std::string mid(const std::string &s, int start, int length) {
    if (start - 1 >= (int)s.size()) return "";
    return s.substr(start - 1, length);
}

// --------------------------
// Fonctions de traitement
// --------------------------

// 1. add_sigma.bas
void add_sigma(const std::string &inputFilename, const std::string &outputFilename, double D) {
    std::ifstream infile(inputFilename);
    if (!infile)
        throw std::runtime_error("Erreur lors de l'ouverture du fichier d'entrée: " + inputFilename);
    std::ofstream outfile(outputFilename);
    if (!outfile)
        throw std::runtime_error("Erreur lors de l'ouverture du fichier de sortie: " + outputFilename);
    
    std::string line;
    // Ignorer les 4 premières lignes
    for (int i = 0; i < 4 && std::getline(infile, line); i++);
    
    const double pi = 3.14159;
    while (std::getline(infile, line)) {
        if (line.empty()) continue;
        std::string TT_str = mid(line, 1, 15);
        std::string I_str  = mid(line, 16, 15);
        double TT = std::stod(TT_str);
        double I  = std::stod(I_str);
        if (I > 0) {
            double N = 2 * pi * D * std::tan(pi * TT / 180.0) / 0.150;
            double sigma = std::sqrt(I / N);
            outfile << line << " " << sigma << "\n";
        }
    }
    infile.close();
    outfile.close();
}

// Fonction add_sigmas_LEO modifiée
void add_sigmas_LEO(const std::string &inputFilename, const std::string &outputFilename, double D) {
    std::ifstream infile(inputFilename);
    if (!infile)
        throw std::runtime_error("Erreur lors de l'ouverture du fichier d'entrée: " + inputFilename);
    std::ofstream outfile(outputFilename);
    if (!outfile)
        throw std::runtime_error("Erreur lors de l'ouverture du fichier de sortie: " + outputFilename);

    std::string line;
    for (int i = 0; i < 4 && std::getline(infile, line); i++);

    const double L = 0.150;
    const double pi = 3.14159;

    std::string line1;
    if (!std::getline(infile, line1))
        throw std::runtime_error("Données insuffisantes dans le fichier.");
    double TT1 = std::stod(mid(line1, 1, 15));

    while (std::getline(infile, line)) {
        if (line.empty()) continue;
        std::string TT_str = mid(line, 1, 15);
        std::string I_str  = mid(line, 16, 15);
        double TT = std::stod(TT_str);
        double I  = std::stod(I_str);
        double S = TT - TT1;
        TT1 = TT;
        if (I > 0) {
            double sigma = std::sqrt((I * L * L) /
                           (pi * D * D * sin(TT / 57.3) * sin(S / 57.3) * (1 + pow(cos(TT / 57.3), 2))));
            outfile << line << " " << sigma << "\n";
        }
    }
    infile.close();
    outfile.close();
}

// 3. add_sigmas_series.bas
void add_sigmas_series(const std::string &setname, double D, int firstFile, int lastFile) {
    std::string bufFilename = setname + "_s.buf";
    std::ofstream bufFile(bufFilename);
    if (!bufFile)
        throw std::runtime_error("Erreur lors de la création du fichier buffer: " + bufFilename);
    
    const double pi = 3.14159;
    
    for (int currentfile = firstFile; currentfile <= lastFile; ++currentfile) {
        std::ostringstream oss;
        oss << std::setw(3) << std::setfill('0') << currentfile;
        std::string suffix = oss.str();
        
        std::string inputFilename  = setname + "_" + suffix + ".chi";
        std::string outputFilename = setname + "_s" + std::to_string(currentfile) + ".dat";
        
        std::ifstream infile(inputFilename);
        if (!infile) {
            // On peut continuer en affichant une erreur
            throw std::runtime_error("Erreur lors de l'ouverture de " + inputFilename);
        }
        std::ofstream outfile(outputFilename);
        if (!outfile) {
            throw std::runtime_error("Erreur lors de l'ouverture de " + outputFilename);
        }
        
        std::string line;
        // Ignorer les 4 premières lignes
        for (int i = 0; i < 4 && std::getline(infile, line); i++);
        
        while (std::getline(infile, line)) {
            if (line.empty()) continue;
            std::string TT_str = mid(line, 1, 15);
            std::string I_str  = mid(line, 16, 15);
            double TT = std::stod(TT_str);
            double I  = std::stod(I_str);
            if (I > 0) {
                double N = 2 * pi * D * std::tan(pi * TT / 180.0) / 0.150;
                double sigma = std::sqrt(I / N);
                outfile << line << " " << sigma << "\n";
            }
        }
        infile.close();
        outfile.close();
        bufFile << outputFilename << "\n";
    }
    bufFile.close();
}

// 4. normalize.bas
// Crée un fichier de normalisation à partir des fichiers .mar2300, puis normalise les fichiers .dat
void normalize_files(const std::string &setname, int firstFile, int lastFile) {
    std::string normFilename = setname + ".norm";
    std::ofstream normFile(normFilename);
    if (!normFile)
        throw std::runtime_error("Erreur lors de l'ouverture du fichier norm pour écriture.");
    
    double Norma = 0.0;
    bool firstNormSet = false;
    const int padWidth = 3;
    
    // Création de la table de normalisation à partir des fichiers .mar2300
    for (int currentfile = firstFile; currentfile <= lastFile; ++currentfile) {
        std::ostringstream oss;
        oss << std::setw(padWidth) << std::setfill('0') << currentfile;
        std::string suffix = oss.str();
        
        std::string marFilename = setname + "_" + suffix + ".mar2300";
        std::ifstream marFile(marFilename);
        if (!marFile)
            continue;
        
        std::string line;
        std::string PARVAL_str;
        while (std::getline(marFile, line)) {
            if (line.size() >= 17 && line.substr(0, 17) == "OUNTS         AVE") {
                if (line.size() >= 23)
                    PARVAL_str = line.substr(17, 6);
                break;
            }
        }
        marFile.close();
        if (!PARVAL_str.empty()) {
            normFile << PARVAL_str << "\n";
            double PARVAL = std::stod(PARVAL_str);
            if (!firstNormSet) {
                Norma = PARVAL;
                firstNormSet = true;
            }
        }
    }
    normFile.close();
    
    // Normalisation des fichiers .dat
    std::string bufFilename = setname + "_sn.buf";
    std::ofstream bufFile(bufFilename);
    if (!bufFile)
        throw std::runtime_error("Erreur lors de la création du fichier buffer: " + bufFilename);
    
    std::ifstream normFileIn(normFilename);
    if (!normFileIn)
        throw std::runtime_error("Erreur lors de l'ouverture du fichier norm pour lecture.");
    
    for (int currentfile = firstFile; currentfile <= lastFile; ++currentfile) {
        std::ostringstream oss;
        oss << std::setw(padWidth) << std::setfill('0') << currentfile;
        std::string suffix = oss.str();
        
        std::string datFilename = setname + "_s" + std::to_string(currentfile) + ".dat";
        std::ifstream datFile(datFilename);
        if (!datFile) {
            std::string dummy;
            std::getline(normFileIn, dummy);
            continue;
        }
        std::string outDatFilename = setname + "_sn" + std::to_string(currentfile) + ".dat";
        std::ofstream outDatFile(outDatFilename);
        if (!outDatFile) {
            std::string dummy;
            std::getline(normFileIn, dummy);
            continue;
        }
        
        std::string normLine;
        if (!std::getline(normFileIn, normLine))
            throw std::runtime_error("Pas assez de valeurs de normalisation.");
        double PARVAL = std::stod(normLine);
        double Normalizer = PARVAL / Norma;
        
        std::string line;
        while (std::getline(datFile, line)) {
            if (line.size() < 31) continue;
            std::string TT_str = mid(line, 1, 15);
            std::string I_str  = mid(line, 16, 15);
            std::string S_str  = mid(line, 31, 10);
            double I = std::stod(I_str);
            double S = std::stod(S_str);
            I *= Normalizer;
            S *= Normalizer;
            outDatFile << TT_str << " " << I << " " << S << "\n";
        }
        datFile.close();
        outDatFile.close();
        bufFile << outDatFilename << "\n";
    }
    normFileIn.close();
    bufFile.close();
}

// 5. scale_series.bas
// Scaler les fichiers .dat (format 2theta-I-sigma) pour qu'ils aient la même intensité totale dans une plage de 2Theta donnée
void scale_series(const std::string &setname, int firstFile, int lastFile, double TTlow, double TThigh) {
    std::string bufFilename = setname + "_ss.buf";
    std::ofstream bufFile(bufFilename);
    if (!bufFile)
        throw std::runtime_error("Erreur lors de la création du fichier buffer: " + bufFilename);
    
    for (int currentfile = firstFile; currentfile <= lastFile; ++currentfile) {
        std::string datFilename = setname + "_s" + std::to_string(currentfile) + ".dat";
        std::ifstream datFile(datFilename);
        if (!datFile)
            throw std::runtime_error("Erreur lors de l'ouverture de " + datFilename);
        
        double ITotal = 0.0;
        std::string line;
        while (std::getline(datFile, line)) {
            if (line.size() < 30) continue;
            double TT = std::stod(mid(line, 1, 15));
            double I  = std::stod(mid(line, 16, 15));
            if (TT >= TTlow && TT <= TThigh)
                ITotal += I;
            if (TT > TThigh)
                break;
        }
        datFile.close();
        
        double Normalizer = 1e6 / ITotal;
        
        std::ifstream datFile2(datFilename);
        if (!datFile2)
            throw std::runtime_error("Erreur lors de la réouverture de " + datFilename);
        std::string outFilename = setname + "_ss" + std::to_string(currentfile) + ".dat";
        std::ofstream outFile(outFilename);
        if (!outFile)
            throw std::runtime_error("Erreur lors de l'ouverture de " + outFilename);
        
        while (std::getline(datFile2, line)) {
            if (line.size() < 31) continue;
            std::string TT_str = mid(line, 1, 15);
            std::string I_str  = mid(line, 16, 15);
            std::string S_str  = mid(line, 31, 10);
            double I_val = std::stod(I_str);
            double S_val = std::stod(S_str);
            I_val *= Normalizer;
            S_val *= Normalizer;
            outFile << TT_str << " " << I_val << " " << S_val << "\n";
        }
        datFile2.close();
        outFile.close();
        bufFile << outFilename << "\n";
    }
    bufFile.close();
}

// 6. seq_scales.bas
// Extrait le facteur d'échelle depuis un fichier .seq de Fullprof et écrit les résultats dans un fichier .txt
void seq_scales(const std::string &setname) {
    std::string seqFilename = setname + ".seq";
    std::string txtFilename = setname + ".txt";
    
    std::ifstream seqFile(seqFilename);
    if (!seqFile)
        throw std::runtime_error("Erreur lors de l'ouverture de " + seqFilename);
    std::ofstream txtFile(txtFilename);
    if (!txtFile)
        throw std::runtime_error("Erreur lors de l'ouverture de " + txtFilename);
    
    std::string line;
    while (std::getline(seqFile, line)) {
        if (line.size() >= 6 && line.substr(0, 6) == "PARVAL") {
            std::string PARVAL_str = mid(line, 7, 6);
            double PARVAL = std::stod(PARVAL_str);
            if (!std::getline(seqFile, line))
                break;
            std::string CHI2_str = mid(line, 13, 14);
            double CHI2 = std::stod(CHI2_str);
            bool found = false;
            while (std::getline(seqFile, line)) {
                if (line.size() >= 26) {
                    std::string paramName = mid(line, 7, 20);
                    if (paramName == "      Scale_ph1_pat1") {
                        found = true;
                        std::string Parameter_str = mid(line, 27, 18);
                        std::string Sigma_str     = mid(line, 45, 18);
                        double Parameter = std::stod(Parameter_str);
                        double Sigma     = std::stod(Sigma_str);
                        if (CHI2 < 1000 && (((int)PARVAL - 12) % 12 == 0))
                            txtFile << PARVAL << " " << Parameter << " " << Sigma << "\n";
                        break;
                    }
                }
            }
        }
    }
    seqFile.close();
    txtFile.close();
}

// 7. shorter_buffer.bas
// Réduit la liste d'un fichier buffer en ne gardant qu'une ligne sur 'factor'
void shorter_buffer(const std::string &setname, int factor) {
    std::string bufFilename = setname + ".buf";
    std::string outFilename = setname + std::to_string(factor) + ".buf";
    
    std::ifstream bufFile(bufFilename);
    if (!bufFile)
        throw std::runtime_error("Erreur lors de l'ouverture de " + bufFilename);
    std::ofstream outFile(outFilename);
    if (!outFile)
        throw std::runtime_error("Erreur lors de l'ouverture de " + outFilename);
    
    std::string line;
    int linenumber = 1;
    while (std::getline(bufFile, line)) {
        if (linenumber % factor == 1)
            outFile << line << "\n";
        ++linenumber;
    }
    bufFile.close();
    outFile.close();
}

// --------------------------
// Classes d'interface graphique
// --------------------------

// Onglet "Add Sigma" déjà présenté précédemment
class AddSigmaTab : public QWidget {
    Q_OBJECT
public:
    AddSigmaTab(QWidget *parent = nullptr) : QWidget(parent) {
        QVBoxLayout* mainLayout = new QVBoxLayout(this);
        
        // Fichier d'entrée
        QHBoxLayout* inputLayout = new QHBoxLayout();
        QLabel* inputLabel = new QLabel("Input File:");
        inputLineEdit = new QLineEdit();
        QPushButton* browseInputBtn = new QPushButton("Browse");
        inputLayout->addWidget(inputLabel);
        inputLayout->addWidget(inputLineEdit);
        inputLayout->addWidget(browseInputBtn);
        mainLayout->addLayout(inputLayout);
        
        // Fichier de sortie
        QHBoxLayout* outputLayout = new QHBoxLayout();
        QLabel* outputLabel = new QLabel("Output File:");
        outputLineEdit = new QLineEdit();
        QPushButton* browseOutputBtn = new QPushButton("Browse");
        outputLayout->addWidget(outputLabel);
        outputLayout->addWidget(outputLineEdit);
        outputLayout->addWidget(browseOutputBtn);
        mainLayout->addLayout(outputLayout);
        
	// Distance du détecteur
	QHBoxLayout* distanceLayout = new QHBoxLayout();
	QLabel* distanceLabel = new QLabel("Detector Distance (mm):");
	distanceLineEdit = new QLineEdit();
	distanceLayout->addWidget(distanceLabel);
	distanceLayout->addWidget(distanceLineEdit);
	mainLayout->addLayout(distanceLayout);
        QPushButton* runButton = new QPushButton("Run");
        mainLayout->addWidget(runButton);
        
        connect(browseInputBtn, &QPushButton::clicked, this, [this](){
            QString fileName = QFileDialog::getOpenFileName(this, "Select Input File");
            if (!fileName.isEmpty())
                inputLineEdit->setText(fileName);
        });
        connect(browseOutputBtn, &QPushButton::clicked, this, [this](){
            QString fileName = QFileDialog::getSaveFileName(this, "Select Output File");
            if (!fileName.isEmpty())
                outputLineEdit->setText(fileName);
        });
	connect(runButton, &QPushButton::clicked, this, [this](){
   		 QString inFile = inputLineEdit->text();
   		 QString outFile = outputLineEdit->text();
   		 if (inFile.isEmpty() || outFile.isEmpty()) {
        		QMessageBox::warning(this, "Error", "Please specify both input and output files.");
       			 return;
    		}
	bool ok;
	double D = distanceLineEdit->text().toDouble(&ok); // Utilisation de distanceLineEdit ici
	if (!ok) {
		QMessageBox::warning(this, "Error", "Invalid detector distance.");
	       	return;
	}
   	try {
        	add_sigma(inFile.toStdString(), outFile.toStdString(), D);
        	QMessageBox::information(this, "Success", "Processing completed successfully.");
    	} catch (std::exception &e) {
        	QMessageBox::critical(this, "Error", e.what());
    	}
	});
}
private:
    QLineEdit *inputLineEdit, *outputLineEdit;
    QLineEdit *distanceLineEdit;
};

// Onglet "Add Sigmas LEO" modifié
class AddSigmasLEOTab : public QWidget {
    Q_OBJECT
public:
    AddSigmasLEOTab(QWidget *parent = nullptr) : QWidget(parent) {
        QVBoxLayout* mainLayout = new QVBoxLayout(this);

        QHBoxLayout* inputLayout = new QHBoxLayout();
        QLabel* inputLabel = new QLabel("Input File:");
        inputLineEdit = new QLineEdit();
        QPushButton* browseInputBtn = new QPushButton("Browse");
        inputLayout->addWidget(inputLabel);
        inputLayout->addWidget(inputLineEdit);
        inputLayout->addWidget(browseInputBtn);
        mainLayout->addLayout(inputLayout);

        QHBoxLayout* outputLayout = new QHBoxLayout();
        QLabel* outputLabel = new QLabel("Output File:");
        outputLineEdit = new QLineEdit();
        QPushButton* browseOutputBtn = new QPushButton("Browse");
        outputLayout->addWidget(outputLabel);
        outputLayout->addWidget(outputLineEdit);
        outputLayout->addWidget(browseOutputBtn);
        mainLayout->addLayout(outputLayout);

        QHBoxLayout* distanceLayout = new QHBoxLayout();
        QLabel* distanceLabel = new QLabel("Detector Distance (mm):");
        distanceLineEdit = new QLineEdit();
        distanceLayout->addWidget(distanceLabel);
        distanceLayout->addWidget(distanceLineEdit);
        mainLayout->addLayout(distanceLayout);

        QPushButton* runButton = new QPushButton("Run");
        mainLayout->addWidget(runButton);

        connect(browseInputBtn, &QPushButton::clicked, this, [this](){
            QString fileName = QFileDialog::getOpenFileName(this, "Select Input File");
            if (!fileName.isEmpty())
                inputLineEdit->setText(fileName);
        });
        connect(browseOutputBtn, &QPushButton::clicked, this, [this](){
            QString fileName = QFileDialog::getSaveFileName(this, "Select Output File");
            if (!fileName.isEmpty())
                outputLineEdit->setText(fileName);
        });
        connect(runButton, &QPushButton::clicked, this, [this](){
            QString inFile = inputLineEdit->text();
            QString outFile = outputLineEdit->text();
            bool ok;
            double D = distanceLineEdit->text().toDouble(&ok);
            if (inFile.isEmpty() || outFile.isEmpty() || !ok) {
                QMessageBox::warning(this, "Error", "Please specify all fields correctly.");
                return;
            }
            try {
                add_sigmas_LEO(inFile.toStdString(), outFile.toStdString(), D);
                QMessageBox::information(this, "Success", "Processing completed successfully.");
            } catch (std::exception &e) {
                QMessageBox::critical(this, "Error", e.what());
            }
        });
    }
private:
    QLineEdit *inputLineEdit, *outputLineEdit, *distanceLineEdit;
};

// Onglet "Add Sigmas Series"
// Onglet "Add Sigmas Series" avec sélection de dossier
class AddSigmasSeriesTab : public QWidget {
    Q_OBJECT
public:
    AddSigmasSeriesTab(QWidget *parent = nullptr) : QWidget(parent) {
        QVBoxLayout* mainLayout = new QVBoxLayout(this);

        // Directory selection
        QHBoxLayout* dirLayout = new QHBoxLayout();
        QLabel* dirLabel = new QLabel("Data Directory:");
        dirLineEdit = new QLineEdit();
        QPushButton* browseDirBtn = new QPushButton("Browse...");
        dirLayout->addWidget(dirLabel);
        dirLayout->addWidget(dirLineEdit);
        dirLayout->addWidget(browseDirBtn);
        mainLayout->addLayout(dirLayout);

        // Base filename
        QHBoxLayout* baseLayout = new QHBoxLayout();
        QLabel* baseLabel = new QLabel("Base Filename:");
        baseLineEdit = new QLineEdit();
        baseLayout->addWidget(baseLabel);
        baseLayout->addWidget(baseLineEdit);
        mainLayout->addLayout(baseLayout);

        // Sample-to-detector distance
        QHBoxLayout* dLayout = new QHBoxLayout();
        QLabel* dLabel = new QLabel("Detector Distance (mm):");
        dLineEdit = new QLineEdit();
        dLayout->addWidget(dLabel);
        dLayout->addWidget(dLineEdit);
        mainLayout->addLayout(dLayout);

        // First file number
        QHBoxLayout* firstLayout = new QHBoxLayout();
        QLabel* firstLabel = new QLabel("First File Number:");
        firstLineEdit = new QLineEdit();
        firstLayout->addWidget(firstLabel);
        firstLayout->addWidget(firstLineEdit);
        mainLayout->addLayout(firstLayout);

        // Last file number
        QHBoxLayout* lastLayout = new QHBoxLayout();
        QLabel* lastLabel = new QLabel("Last File Number:");
        lastLineEdit = new QLineEdit();
        lastLayout->addWidget(lastLabel);
        lastLayout->addWidget(lastLineEdit);
        mainLayout->addLayout(lastLayout);

        QPushButton* runButton = new QPushButton("Run");
        mainLayout->addWidget(runButton);

        // Bouton "Browse" pour choisir le répertoire
        connect(browseDirBtn, &QPushButton::clicked, this, [this](){
            QString dir = QFileDialog::getExistingDirectory(this, "Select Data Directory");
            if (!dir.isEmpty())
                dirLineEdit->setText(dir);
        });

        // Traitement lors du clic sur "Run"
        connect(runButton, &QPushButton::clicked, this, [this](){
            QString dirPath = dirLineEdit->text();
            QString base = baseLineEdit->text();
            bool ok1, ok2, ok3;
            double D = dLineEdit->text().toDouble(&ok1);
            int first = firstLineEdit->text().toInt(&ok2);
            int last = lastLineEdit->text().toInt(&ok3);

            if (dirPath.isEmpty() || base.isEmpty() || !ok1 || !ok2 || !ok3) {
                QMessageBox::warning(this, "Error", "Please fill all fields correctly.");
                return;
            }

            // Se déplacer dans le répertoire choisi
            QDir dir(dirPath);
            QDir::setCurrent(dirPath);

            try {
                add_sigmas_series(base.toStdString(), D, first, last);
                QMessageBox::information(this, "Success", "Add Sigmas Series completed successfully.");
            } catch (std::exception &e) {
                QMessageBox::critical(this, "Error", e.what());
            }
        });
    }

private:
    QLineEdit *dirLineEdit, *baseLineEdit, *dLineEdit, *firstLineEdit, *lastLineEdit;
};

// Onglet "Normalize"
class NormalizeTab : public QWidget {
    Q_OBJECT
public:
    NormalizeTab(QWidget *parent = nullptr) : QWidget(parent) {
        QVBoxLayout* mainLayout = new QVBoxLayout(this);
        
        // Base filename
        QHBoxLayout* baseLayout = new QHBoxLayout();
        QLabel* baseLabel = new QLabel("Base Filename:");
        baseLineEdit = new QLineEdit();
        QPushButton* browseButton = new QPushButton("Browse Directory");
        baseLayout->addWidget(baseLabel);
        baseLayout->addWidget(baseLineEdit);
        baseLayout->addWidget(browseButton);
        mainLayout->addLayout(baseLayout);
        
        // First file number
        QHBoxLayout* firstLayout = new QHBoxLayout();
        QLabel* firstLabel = new QLabel("First File Number:");
        firstLineEdit = new QLineEdit();
        firstLayout->addWidget(firstLabel);
        firstLayout->addWidget(firstLineEdit);
        mainLayout->addLayout(firstLayout);
        
        // Last file number
        QHBoxLayout* lastLayout = new QHBoxLayout();
        QLabel* lastLabel = new QLabel("Last File Number:");
        lastLineEdit = new QLineEdit();
        lastLayout->addWidget(lastLabel);
        lastLayout->addWidget(lastLineEdit);
        mainLayout->addLayout(lastLayout);
        
        QPushButton* runButton = new QPushButton("Run");
        mainLayout->addWidget(runButton);

        connect(browseButton, &QPushButton::clicked, this, [this](){
            QString dir = QFileDialog::getExistingDirectory(this, "Select Directory");
            if (!dir.isEmpty()) {
                baseLineEdit->setText(QDir(dir).absolutePath() + "/");
            }
        });

        connect(runButton, &QPushButton::clicked, this, [this](){
            QString base = baseLineEdit->text();
            bool ok1, ok2;
            int first = firstLineEdit->text().toInt(&ok1);
            int last = lastLineEdit->text().toInt(&ok2);
            if (base.isEmpty() || !ok1 || !ok2) {
                QMessageBox::warning(this, "Error", "Please fill all fields correctly.");
                return;
            }
            try {
                normalize_files(base.toStdString(), first, last);
                QMessageBox::information(this, "Success", "Normalization completed successfully.");
            } catch (std::exception &e) {
                QMessageBox::critical(this, "Error", e.what());
            }
        });
    }
private:
    QLineEdit *baseLineEdit, *firstLineEdit, *lastLineEdit;
};

// Onglet "Scale Series"
class ScaleSeriesTab : public QWidget {
    Q_OBJECT
public:
    ScaleSeriesTab(QWidget *parent = nullptr) : QWidget(parent) {
        QVBoxLayout* mainLayout = new QVBoxLayout(this);
        
        // Base filename
        QHBoxLayout* baseLayout = new QHBoxLayout();
        QLabel* baseLabel = new QLabel("Base Filename:");
        baseLineEdit = new QLineEdit();
        QPushButton* browseButton = new QPushButton("Browse Directory");
        baseLayout->addWidget(baseLabel);
        baseLayout->addWidget(baseLineEdit);
        baseLayout->addWidget(browseButton);
        mainLayout->addLayout(baseLayout);
        
        // First file number
        QHBoxLayout* firstLayout = new QHBoxLayout();
        QLabel* firstLabel = new QLabel("First File Number:");
        firstLineEdit = new QLineEdit();
        firstLayout->addWidget(firstLabel);
        firstLayout->addWidget(firstLineEdit);
        mainLayout->addLayout(firstLayout);
        
        // Last file number
        QHBoxLayout* lastLayout = new QHBoxLayout();
        QLabel* lastLabel = new QLabel("Last File Number:");
        lastLineEdit = new QLineEdit();
        lastLayout->addWidget(lastLabel);
        lastLayout->addWidget(lastLineEdit);
        mainLayout->addLayout(lastLayout);
        
        // TT low
        QHBoxLayout* ttLowLayout = new QHBoxLayout();
        QLabel* ttLowLabel = new QLabel("Lower 2Theta:");
        ttLowLineEdit = new QLineEdit();
        ttLowLayout->addWidget(ttLowLabel);
        ttLowLayout->addWidget(ttLowLineEdit);
        mainLayout->addLayout(ttLowLayout);
        
        // TT high
        QHBoxLayout* ttHighLayout = new QHBoxLayout();
        QLabel* ttHighLabel = new QLabel("Upper 2Theta:");
        ttHighLineEdit = new QLineEdit();
        ttHighLayout->addWidget(ttHighLabel);
        ttHighLayout->addWidget(ttHighLineEdit);
        mainLayout->addLayout(ttHighLayout);
        
        QPushButton* runButton = new QPushButton("Run");
        mainLayout->addWidget(runButton);

        connect(browseButton, &QPushButton::clicked, this, [this](){
            QString dir = QFileDialog::getExistingDirectory(this, "Select Directory");
            if (!dir.isEmpty()) {
                baseLineEdit->setText(QDir(dir).absolutePath() + "/");
            }
        });

        connect(runButton, &QPushButton::clicked, this, [this](){
            QString base = baseLineEdit->text();
            bool ok1, ok2, ok3, ok4;
            int first = firstLineEdit->text().toInt(&ok1);
            int last = lastLineEdit->text().toInt(&ok2);
            double TTlow = ttLowLineEdit->text().toDouble(&ok3);
            double TThigh = ttHighLineEdit->text().toDouble(&ok4);
            if (base.isEmpty() || !ok1 || !ok2 || !ok3 || !ok4) {
                QMessageBox::warning(this, "Error", "Please fill all fields correctly.");
                return;
            }
            try {
                scale_series(base.toStdString(), first, last, TTlow, TThigh);
                QMessageBox::information(this, "Success", "Scale Series completed successfully.");
            } catch (std::exception &e) {
                QMessageBox::critical(this, "Error", e.what());
            }
        });
    }
private:
    QLineEdit *baseLineEdit, *firstLineEdit, *lastLineEdit, *ttLowLineEdit, *ttHighLineEdit;
};

// Onglet "Seq Scales"
class SeqScalesTab : public QWidget {
    Q_OBJECT
public:
    SeqScalesTab(QWidget *parent = nullptr) : QWidget(parent) {
        QVBoxLayout* mainLayout = new QVBoxLayout(this);

        // Base filename
        QHBoxLayout* baseLayout = new QHBoxLayout();
        QLabel* baseLabel = new QLabel("Base Filename (without extension):");
        baseLineEdit = new QLineEdit();
        baseLayout->addWidget(baseLabel);
        baseLayout->addWidget(baseLineEdit);
        mainLayout->addLayout(baseLayout);

        // Bouton de sélection de répertoire
        QPushButton* browseButton = new QPushButton("Browse Directory");
        mainLayout->addWidget(browseButton);

        connect(browseButton, &QPushButton::clicked, this, [this](){
            QString dirPath = QFileDialog::getExistingDirectory(this, "Select Directory");
            if (!dirPath.isEmpty()) {
                QDir dir(dirPath);
                baseLineEdit->setText(dir.dirName());
            }
        });

        QPushButton* runButton = new QPushButton("Run");
        mainLayout->addWidget(runButton);

        connect(runButton, &QPushButton::clicked, this, [this](){
            QString base = baseLineEdit->text();
            if (base.isEmpty()) {
                QMessageBox::warning(this, "Error", "Please specify the base filename.");
                return;
            }
            try {
                seq_scales(base.toStdString());
                QMessageBox::information(this, "Success", "Seq Scales completed successfully.");
            } catch (std::exception &e) {
                QMessageBox::critical(this, "Error", e.what());
            }
        });
    }
private:
    QLineEdit *baseLineEdit;
};

// Onglet "Shorter Buffer"
class ShorterBufferTab : public QWidget {
    Q_OBJECT
public:
    ShorterBufferTab(QWidget *parent = nullptr) : QWidget(parent) {
        QVBoxLayout* mainLayout = new QVBoxLayout(this);

        // Base filename
        QHBoxLayout* baseLayout = new QHBoxLayout();
        QLabel* baseLabel = new QLabel("Filename (without extension):");
        baseLineEdit = new QLineEdit();
        QPushButton* browseBtn = new QPushButton("Browse Directory");
        baseLayout->addWidget(baseLabel);
        baseLayout->addWidget(baseLineEdit);
        baseLayout->addWidget(browseBtn);
        mainLayout->addLayout(baseLayout);

        // Factor
        QHBoxLayout* factorLayout = new QHBoxLayout();
        QLabel* factorLabel = new QLabel("Factor:");
        factorLineEdit = new QLineEdit();
        factorLayout->addWidget(factorLabel);
        factorLayout->addWidget(factorLineEdit);
        mainLayout->addLayout(factorLayout);

        QPushButton* runButton = new QPushButton("Run");
        mainLayout->addWidget(runButton);

        connect(browseBtn, &QPushButton::clicked, this, [this](){
            QString directory = QFileDialog::getExistingDirectory(this, "Select Directory", "");
            if (!directory.isEmpty()) {
                baseLineEdit->setText(directory + "/");
            }
        });

        connect(runButton, &QPushButton::clicked, this, [this](){
            QString base = baseLineEdit->text();
            bool ok;
            int factor = factorLineEdit->text().toInt(&ok);
            if (base.isEmpty() || !ok) {
                QMessageBox::warning(this, "Error", "Please fill all fields correctly.");
                return;
            }
            try {
                shorter_buffer(base.toStdString(), factor);
                QMessageBox::information(this, "Success", "Shorter Buffer completed successfully.");
            } catch (std::exception &e) {
                QMessageBox::critical(this, "Error", e.what());
            }
        });
    }
private:
    QLineEdit *baseLineEdit, *factorLineEdit;
};

// Onglet "Help"
class HelpTab : public QWidget {
    Q_OBJECT
public:
    HelpTab(QWidget *parent = nullptr) : QWidget(parent) {
        QVBoxLayout *layout = new QVBoxLayout(this);

        QTextEdit *helpViewer = new QTextEdit(this);
        helpViewer->setReadOnly(true);

	QFile helpFile(":/help.txt");
	if (helpFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
    		QTextStream stream(&helpFile);
		helpViewer->setPlainText(stream.readAll());
    		helpFile.close();
	} else {
    		helpViewer->setPlainText("Impossible de charger le fichier d'aide.");
	}

        layout->addWidget(helpViewer);
    }
};

// --------------------------
// Fenêtre principale
// --------------------------
class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr) : QMainWindow(parent) {
        QTabWidget* tabWidget = new QTabWidget(this);
        tabWidget->addTab(new AddSigmaTab(this), "Add Sigma");
        tabWidget->addTab(new AddSigmasLEOTab(this), "Add Sigmas LEO");
        tabWidget->addTab(new AddSigmasSeriesTab(this), "Add Sigmas Series");
        tabWidget->addTab(new NormalizeTab(this), "Normalize");
        tabWidget->addTab(new ScaleSeriesTab(this), "Scale Series");
        tabWidget->addTab(new SeqScalesTab(this), "Seq Scales");
        tabWidget->addTab(new ShorterBufferTab(this), "Shorter Buffer");
        tabWidget->addTab(new HelpTab(), "Help");

        setCentralWidget(tabWidget);
        setWindowTitle("Powder Data Processing");
        resize(700, 500);
    }
};

#include "powder_processing.moc"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    MainWindow mainWin;
    mainWin.show();
    return app.exec();
}

