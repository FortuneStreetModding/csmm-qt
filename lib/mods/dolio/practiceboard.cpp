#include "practiceboard.h"
#include "lib/powerpcasm.h"

void PracticeBoard::writeAsm(QDataStream &stream, const AddressMapper &addressMapper, const QVector<MapDescriptor> &mapDescriptors) {
    short easyPracticeBoard = -1;
    short standardPracticeBoard = -1;

    QStringList errorMsgs;

    getPracticeBoards(mapDescriptors, easyPracticeBoard, standardPracticeBoard, errorMsgs);

    // li r0,0x29                                                                 -> li r0,easyPracticeBoard
    stream.device()->seek(addressMapper.boomToFileAddress(0x80173bf8)); stream << PowerPcAsm::li(0, easyPracticeBoard);
    // li r0,0x14                                                                 -> li r0,standardPracticeBoard
    stream.device()->seek(addressMapper.boomToFileAddress(0x80173c04)); stream << PowerPcAsm::li(0, standardPracticeBoard);
}

void PracticeBoard::readAsm(QDataStream &stream, const AddressMapper &addressMapper, QVector<MapDescriptor> &mapDescriptors) {
    stream.device()->seek(addressMapper.boomToFileAddress(0x80173bf8 + 2));
    qint16 easyPracticeBoard; stream >> easyPracticeBoard;
    stream.device()->seek(addressMapper.boomToFileAddress(0x80173c04 + 2));
    qint16 standardPracticeBoard; stream >> standardPracticeBoard;

    mapDescriptors[easyPracticeBoard].isPracticeBoard = true;
    mapDescriptors[easyPracticeBoard].mapSet = 0;
    mapDescriptors[standardPracticeBoard].isPracticeBoard = true;
    mapDescriptors[standardPracticeBoard].mapSet = 1;
}
