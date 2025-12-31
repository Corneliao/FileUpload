import QtQuick
import file_upload
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import QtQuick.Controls.Material

QuickFramelessWindow {
    width: 340
    height: 480
    Component.onCompleted: {
        setWindowTitleBar(title_bar_layout);
        moveCenter(true);
    }

    ColumnLayout {
        anchors.fill: parent
        Item {
            id: title
            Layout.alignment: Qt.AlignTop
            Layout.preferredHeight: 60
            Layout.maximumHeight: 60
            Layout.fillWidth: true

            RowLayout {
                id: title_bar_layout
                anchors.fill: parent
                anchors.leftMargin: 12
                anchors.rightMargin: 12
                Label {
                    text: "Transfer"
                    font.pixelSize: 16
                }

                Loader {
                    Layout.fillWidth: true
                }
            }
        }

        Rectangle {
            Layout.preferredWidth: parent.width - 40
            Layout.preferredHeight: 200
            Layout.alignment: Qt.AlignHCenter
            border.color: Qt.color("lightgray")
            radius: 10
            states: [
                State {
                    when: files_view.count <= 0
                    PropertyChanges {
                        files_view {
                            visible: false
                        }
                        ico {
                            visible: true
                        }
                    }
                },
                State {
                    when: files_view.count > 0
                    PropertyChanges {
                        files_view {
                            visible: true
                        }
                        ico {
                            visible: false
                        }
                    }
                }
            ]

            FileDialog {
                id: file_dialog
                fileMode: FileDialog.OpenFile
                onAccepted: {
                    const result_path = selectedFile.toString();
                    var data = Controller.parseFile(result_path.slice(8));
                    let msg_id = Controller.generatorUuid();
                    file_model.append({
                        "fileName": data.fileName,
                        "fileSize": data.fileSize,
                        "filePath": data.filePath,
                        "msg_id": msg_id
                    });

                    //Controller.sendFile(data);
                }
            }

            ListModel {
                id: file_model
            }

            ListView {
                id: files_view
                anchors.fill: parent
                anchors.margins: 5
                model: file_model
                spacing: 8
                clip: true
                delegate: Item {
                    id: file_delegate
                    required property string fileName
                    required property string fileSize
                    required property string filePath
                    required property int index
                    required property string msg_id
                    width: ListView.view.width
                    height: 60

                    Component.onCompleted: {
                        Qt.callLater(function () {
                            Controller.insertItem(msg_id, progress_bar);
                            const data = {
                                "fileName": fileName,
                                "fileSize": fileSize,
                                "filePath": filePath,
                                "msg_id": msg_id
                            };

                            Controller.sendFile(data);
                        });
                    }

                    Rectangle {
                        anchors.fill: parent
                        radius: 7
                        color: Qt.color("#2196F3")
                        RowLayout {
                            spacing: 8
                            anchors {
                                fill: parent
                                leftMargin: 12
                                rightMargin: 12
                            }

                            Image {
                                source: "fileIco.svg"
                                Layout.preferredWidth: 25
                                Layout.preferredHeight: width
                                antialiasing: true
                                smooth: true
                                mipmap: true
                            }

                            Column {
                                spacing: 8
                                Label {
                                    id: name
                                    text: file_delegate.fileName
                                    font.pixelSize: 12
                                    color: "white"
                                }
                                Row {
                                    spacing: 8
                                    Label {
                                        id: size_
                                        text: file_delegate.fileSize
                                        font.pixelSize: 11
                                        color: "white"
                                    }
                                    ProgressBar {
                                        id: progress_bar
                                        width: 120
                                        from: 0
                                        to: 100
                                        anchors.verticalCenter: parent.verticalCenter
                                        height: 5
                                    }
                                }
                            }

                            Loader {
                                Layout.fillWidth: true
                            }
                        }
                    }
                }
            }

            Image {
                id: ico
                source: "MaterialSymbolsLightAddNotesOutline.svg"
                width: 30
                height: width
                anchors.centerIn: parent
                antialiasing: true
                mipmap: true
                smooth: true
                scale: mouse_area.containsMouse ? 1.1 : 1

                Behavior on scale {
                    NumberAnimation {
                        duration: 200
                    }
                }
            }

            MouseArea {
                id: mouse_area
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor
                hoverEnabled: true
                onClicked: {
                    file_dialog.open();
                }
            }
        }

        Loader {
            Layout.fillHeight: true
        }
    }
}
