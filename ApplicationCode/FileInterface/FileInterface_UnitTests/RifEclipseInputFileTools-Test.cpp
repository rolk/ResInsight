#include "gtest/gtest.h"

#include "RigCaseData.h"

#include "RifEclipseInputFileTools.h"



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RifEclipseInputFileToolsTest, FaultFaces)
{
    {
        QStringList faceTexts;
        faceTexts << "X" << "X+" << "I" << "I+" << "x" << "x+" << "i" << "i+";

        cvf::StructGridInterface::FaceEnum faceType;
        foreach (QString text, faceTexts)
        {
            faceType = RifEclipseInputFileTools::faceEnumFromText(text);
            EXPECT_EQ(cvf::StructGridInterface::POS_I, faceType);
        }
    }

    {
        QStringList faceTexts;
        faceTexts << "X-" << "I-" << "x-" << "i-";

        cvf::StructGridInterface::FaceEnum faceType;
        foreach(QString text, faceTexts)
        {
            faceType = RifEclipseInputFileTools::faceEnumFromText(text);
            EXPECT_EQ(cvf::StructGridInterface::NEG_I, faceType);
        }
    }

    {
        QStringList faceTexts;
        faceTexts << "Y" << "Y+" << "J" << "J+" << "y" << "y+" << "j" << "j+";

        cvf::StructGridInterface::FaceEnum faceType;
        foreach(QString text, faceTexts)
        {
            faceType = RifEclipseInputFileTools::faceEnumFromText(text);
            EXPECT_EQ(cvf::StructGridInterface::POS_J, faceType);
        }
    }

    {
        QStringList faceTexts;
        faceTexts << "Y-" << "J-" << "y-" << "j-";

        cvf::StructGridInterface::FaceEnum faceType;
        foreach(QString text, faceTexts)
        {
            faceType = RifEclipseInputFileTools::faceEnumFromText(text);
            EXPECT_EQ(cvf::StructGridInterface::NEG_J, faceType);
        }
    }

    {
        QStringList faceTexts;
        faceTexts << "Z" << "Z+" << "K" << "k+" << "z" << "z+" << "k" << "k+";

        cvf::StructGridInterface::FaceEnum faceType;
        foreach(QString text, faceTexts)
        {
            faceType = RifEclipseInputFileTools::faceEnumFromText(text);
            EXPECT_EQ(cvf::StructGridInterface::POS_K, faceType);
        }
    }

    {
        QStringList faceTexts;
        faceTexts << "Z-" << "K-" << "z-" << "k-";

        cvf::StructGridInterface::FaceEnum faceType;
        foreach(QString text, faceTexts)
        {
            faceType = RifEclipseInputFileTools::faceEnumFromText(text);
            EXPECT_EQ(cvf::StructGridInterface::NEG_K, faceType);
        }
    }


    // Invalid faces
    {
        QStringList faceTexts;
        faceTexts << "Z--" << "z--" << "-k-" << " -k " << "   +k-  ";

        cvf::StructGridInterface::FaceEnum faceType;
        foreach(QString text, faceTexts)
        {
            faceType = RifEclipseInputFileTools::faceEnumFromText(text);
            EXPECT_EQ(cvf::StructGridInterface::NO_FACE, faceType);
        }
    }

    // Valid cases with whitespace
    {
        QStringList faceTexts;
        faceTexts << " X" << " X+ " << " I " << " i+  ";

        cvf::StructGridInterface::FaceEnum faceType;
        foreach(QString text, faceTexts)
        {
            faceType = RifEclipseInputFileTools::faceEnumFromText(text);
            EXPECT_EQ(cvf::StructGridInterface::POS_I, faceType);
        }
    }

}
