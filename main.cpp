#include "projecthub.h"

#include <QApplication>
#include <QPushButton>
#include <QVBoxLayout>
#include <QSlider>
#include <QListView>


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QApplication::setStyle("Fusion");

    QVector<Project> projects = {
        Project
        {
            .type = ProjectType::QtWidgets,
            .title = "First Project",
            .shortDescription = "This is my first project",
            .description = "This is my first project",
            .launch = []() {
                auto *window = new QWidget();
                window->setWindowTitle("First Project");
                window->setFixedSize(800, 450);
                window->show();
            },
        },
        Project
        {
            .type = ProjectType::Qml,
            .title = "Learn QML",
            .shortDescription = "This is my first QML project",
            .description = "This is my first QML project",
            .launch = []() {
                auto *window = new QWidget();
                window->setWindowTitle("Learn QML");
                window->setFixedSize(800, 450);
                window->show();
            },
        },
        Project
        {
            .type = ProjectType::QtWidgets,
            .title = "Chess Game",
            .shortDescription = "A networked multiplayer chess game.",
            .description = "A networked multiplayer chess game.",
            .launch = []() {
                auto *window = new QWidget();
                window->setWindowTitle("Chess Game");
                window->setFixedSize(800, 450);
                window->show();
            },
        },
    };

    ProjectHub projectHub(projects);
    projectHub.setWindowTitle("Project Hub");
    projectHub.show();

    return app.exec();
}
