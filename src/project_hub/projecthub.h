#ifndef PROJECTHUB_H
#define PROJECTHUB_H

#include <QListWidget>
#include <QMainWindow>
#include <QDate>
#include <QLabel>

#include <functional>

enum class ProjectType
{
    QtWidgets,
    Qml,
};

struct Project
{
    ProjectType type;

    QString title;
    QString shortDescription;
    QString description;

    QDate startDate;
    QDate endDate;

    std::function<void()> launch;
};

class ProjectListTile : public QWidget
{
    Q_OBJECT

public:
    explicit ProjectListTile(const Project& project, QWidget *parent = nullptr);
};

class ProjectHub : public QMainWindow
{
    Q_OBJECT

public:
    explicit ProjectHub(QVector<Project> projects, QWidget *parent = nullptr);

signals:

private slots:
    void onProjectSelected();
    void launchSelectedProject();

    void filterProjects(const QString& text);
private:
    Project *selectedProject();;
    void setupUI();
private:
    QVector<Project> m_projects;

    QLabel *m_titleLabel;
    QLabel *m_descriptionLabel;

    int m_selectedProjectIndex = -1;

    QListWidget *m_projectListWidget;
};

#endif // PROJECTHUB_H
