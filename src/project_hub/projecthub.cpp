#include "projecthub.h"

#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSplitter>
#include <QVBoxLayout>

ProjectHub::ProjectHub(QVector<Project> projects, QWidget *parent)
    : m_projects(std::move(projects)),
    QMainWindow{parent}
{
    setupUI();

    setFixedSize(1280, 720);
}

void ProjectHub::onProjectSelected()
{
    QListWidgetItem *selectedItem = m_projectListWidget->currentItem();
    if(!selectedItem)
    {
        return;
    }

    m_selectedProjectIndex = selectedItem->data(Qt::UserRole).value<int>();

    Project* project = selectedProject();

    if(!project)
    {
        return;
    }

    m_titleLabel->setText(project->title);
    m_descriptionLabel->setText(project->description);
}

void ProjectHub::launchSelectedProject()
{
    Project *project = selectedProject();
    if(!project)
    {
        return;
    }

    project->launch();
}

void ProjectHub::filterProjects(const QString &text)
{
    for (int i = 0; i < m_projectListWidget->count(); ++i) {
        QListWidgetItem *item = m_projectListWidget->item(i);
        int index = item->data(Qt::UserRole).value<int>();

        Project& project = m_projects[index];

        item->setHidden(!project.title.contains(text, Qt::CaseInsensitive));
    }

}

Project *ProjectHub::selectedProject()
{
    if(m_selectedProjectIndex < 0 || m_selectedProjectIndex >= m_projects.count())
    {
        return nullptr;
    }

    return &m_projects[m_selectedProjectIndex];
}

void ProjectHub::setupUI()
{
    // Projects
    auto projectsWidget = new QGroupBox("Projects");
    auto projectsLayout = new QVBoxLayout(projectsWidget);
    m_projectListWidget = new QListWidget();
    connect(m_projectListWidget, &QListWidget::itemSelectionChanged, this, &ProjectHub::onProjectSelected);

    auto projectSearchLineEdit = new QLineEdit();
    projectSearchLineEdit->setPlaceholderText("Search...");
    connect(projectSearchLineEdit, &QLineEdit::textChanged, this, &ProjectHub::filterProjects);
    projectsLayout->addWidget(projectSearchLineEdit);
    projectsLayout->addWidget(m_projectListWidget);
    
    for (size_t i = 0; i < m_projects.count(); ++i) {

        auto projectItem = new QListWidgetItem(m_projectListWidget);
        projectItem->setData(Qt::UserRole, QVariant::fromValue(i));

        auto projectListTile = new ProjectListTile(m_projects[i]);

        projectItem->setSizeHint(projectListTile->sizeHint());

        m_projectListWidget->setItemWidget(projectItem, projectListTile);
    }

    // Details
    auto detailsWidget = new QGroupBox("Details");
    auto detailsBanner = new QWidget();
    detailsBanner->setStyleSheet("background-color: grey");

    QFont titleFont;
    titleFont.setBold(true);
    titleFont.setPointSize(14);
    m_titleLabel = new QLabel("");
    m_titleLabel->setFont(titleFont);
    m_descriptionLabel = new QLabel("");
    QFont descriptionFont;
    descriptionFont.setPointSize(10);
    m_descriptionLabel->setFont(descriptionFont);
    auto detailsLayout = new QVBoxLayout(detailsWidget);
    detailsLayout->addWidget(detailsBanner, 3);

    auto detailsInfoLayout = new QVBoxLayout();
    detailsLayout->addLayout(detailsInfoLayout, 2);

    detailsInfoLayout->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    detailsInfoLayout->addWidget(m_titleLabel);
    detailsInfoLayout->addWidget(m_descriptionLabel);

    QFont launchFont;
    launchFont.setBold(true);
    launchFont.setPointSize(12);
    auto launchButton = new QPushButton("Launch");
    launchButton->setFont(launchFont);
    launchButton->setFixedHeight(40);
    detailsLayout->addWidget(launchButton);
    connect(launchButton, &QPushButton::clicked, this, &ProjectHub::launchSelectedProject);

    //auto splitter = new QSplitter();

    auto centralWidget = new QWidget();
    auto centralLayout = new QHBoxLayout(centralWidget);
    centralLayout->setContentsMargins(10, 10, 10, 10);
    centralLayout->addWidget(projectsWidget, 3);
    centralLayout->addWidget(detailsWidget, 7);

    setCentralWidget(centralWidget);
}

ProjectListTile::ProjectListTile(const Project &project, QWidget *parent)
    : QWidget(parent)
{
    auto outerLayout = new QHBoxLayout(this);

    auto icon = new QLabel(project.type == ProjectType::Qml ? "QML" : "Widgets");

    icon->setStyleSheet("color: green");

    QFont iconFont;
    iconFont.setBold(true);
    iconFont.setPointSize(project.type == ProjectType::Qml ? 14 : 9);

    icon->setFont(iconFont);
    outerLayout->addWidget(icon, 2, Qt::AlignCenter);

    auto innerLayout = new QVBoxLayout();
    outerLayout->addLayout(innerLayout, 8);

    innerLayout->setAlignment(Qt::AlignLeft | Qt::AlignTop);

    QFont titleFont;
    titleFont.setBold(true);
    titleFont.setPointSize(12);
    auto projectTitleLabel = new QLabel(project.title);
    projectTitleLabel->setFont(titleFont);
    innerLayout->addWidget(projectTitleLabel);

    QFont subtitleFont;
    subtitleFont.setItalic(true);
    auto projectSubtitleLabel = new QLabel(project.shortDescription);
    projectSubtitleLabel->setFont(subtitleFont);
    innerLayout->addWidget(projectSubtitleLabel);
}
