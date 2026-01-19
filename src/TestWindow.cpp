#include "TestWindow.h"

TestWindow::TestWindow(QWidget *parent)
    : QWidget(parent)
    , m_quarterWidget(nullptr)
    , m_layout(nullptr)
    , m_rootNode(nullptr)
    , m_textRoot(nullptr)
{
    m_layout = new QVBoxLayout(this);
    setLayout(m_layout);
    
    // Initialize Coin3D
    Quarter::init();

    // Create QuarterWidget
    m_quarterWidget = new QuarterWidget(this);
    m_layout->addWidget(m_quarterWidget);
    
    // Create root nodes
    m_rootNode = new SoSeparator();
    m_textRoot = new SoSeparator();
    m_rootNode->addChild(m_textRoot);
    
    // Set the scene graph
    m_quarterWidget->setSceneGraph(m_rootNode);
}

TestWindow::~TestWindow()
{
}

void TestWindow::initialize()
{
    // Set window properties
    setWindowTitle("Test Window");
    resize(800, 600);
    
    // Set background color
    m_quarterWidget->setBackgroundColor(QColor(230, 230, 230));
}

void TestWindow::draw3DText(const QString &text, float x, float y, float z, float size)
{
    // Create a separator for this text
    SoSeparator *textSep = new SoSeparator();
    
    // Create translation node
    SoTranslation *translation = new SoTranslation();
    translation->translation.setValue(x, y, z);
    textSep->addChild(translation);
    
    // Create font node
    SoFont *font = new SoFont();
    font->name.setValue("Arial");
    font->size.setValue(size);
    textSep->addChild(font);
    
    // Create material node
    SoMaterial *material = new SoMaterial();
    material->diffuseColor.setValue(1.0f, 0.0f, 0.0f); // Red color
    textSep->addChild(material);
    
    // Create text node
    SoText3 *textNode = new SoText3();
    textNode->string.setValue(text.toStdString().c_str());
    textNode->justification.setValue(SoText3::CENTER);
    textSep->addChild(textNode);
    
    // Add to text root
    m_textRoot->addChild(textSep);
    
    // Render the scene
    m_quarterWidget->redraw();
}

void TestWindow::clearAllText()
{
    // Remove all text children
    m_textRoot->removeAllChildren();
    
    // Render the scene
    m_quarterWidget->redraw();
}
