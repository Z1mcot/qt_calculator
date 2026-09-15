import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Qt.labs.settings 1.0

ApplicationWindow {
    id: window
    visible: true
    width: 420
    height: 640
    minimumWidth: 420
    minimumHeight: 560
    title: "Калькулятор"

    Shortcut { sequence: "0"; onActivated: appendDigit("0") }
    Shortcut { sequence: "1"; onActivated: appendDigit("1") }
    Shortcut { sequence: "2"; onActivated: appendDigit("2") }
    Shortcut { sequence: "3"; onActivated: appendDigit("3") }
    Shortcut { sequence: "4"; onActivated: appendDigit("4") }
    Shortcut { sequence: "5"; onActivated: appendDigit("5") }
    Shortcut { sequence: "6"; onActivated: appendDigit("6") }
    Shortcut { sequence: "7"; onActivated: appendDigit("7") }
    Shortcut { sequence: "8"; onActivated: appendDigit("8") }
    Shortcut { sequence: "9"; onActivated: appendDigit("9") }

    Shortcut { sequence: "."; onActivated: appendDot() }
    Shortcut { sequence: ","; onActivated: appendDot() }

    Shortcut { sequence: "+"; onActivated: pressOperator("+") }
    Shortcut { sequence: "-"; onActivated: pressOperator("-") }
    Shortcut { sequence: "*"; onActivated: pressOperator("*") }
    Shortcut { sequence: "/"; onActivated: pressOperator("/") }

    Shortcut { sequence: "="; onActivated: pressEquals() }
    Shortcut { sequence: "Return"; onActivated: pressEquals() }
    Shortcut { sequence: "Enter"; onActivated: pressEquals() }

    Shortcut { sequence: "Escape"; onActivated: resetAll() }
    Shortcut {
        sequence: "Backspace"
        onActivated: {
            if (entry.length > 1) {
                entry = entry.substring(0, entry.length - 1)
            } else {
                entry = "0"
            }
        }
    }

    Settings {
        id: windowSettings
        property alias x: window.x
        property alias y: window.y
        property alias width: window.width
        property alias height: window.height
    }

    // ----- calculator state -----
    property string entry: "0"

    function resetAll() {
        entry = "0"
    }

    function appendDigit(d) {
        if (entry === "0")
            entry = d
        else
            entry += d
    }

    function appendDot() {
        if (entry === "0") {
            entry = "0."
            return
        }
        // ищем начало текущего числового сегмента (после последнего оператора)
        var lastOpIndex = Math.max(
            entry.lastIndexOf("+"), entry.lastIndexOf("-"),
            entry.lastIndexOf("*"), entry.lastIndexOf("/")
        )
        var currentSegment = entry.substring(lastOpIndex + 1)
        if (currentSegment.indexOf(".") !== -1)
            return
        entry += "."
    }

    function appendOperator(op) {
        var trimmed = entry.trim()
        if (trimmed === "")
            return
        if (/[+\-*/]$/.test(trimmed))
            return
        entry = trimmed + " " + op + " "
    }

    function pressOperator(op) {
        if (op === "-" && isOperandStart()) {
            startNegativeOperand()
            return
        }
        appendOperator(op)
    }

    function isOperandStart() {
        if (entry === "0")
            return true
        var trimmed = entry.trim()
        return /[+\-*/]$/.test(trimmed)
    }

    function startNegativeOperand() {
        if (entry === "0") {
            entry = "-"
            return
        }

        if (entry.slice(-1) === "-")
            return

        var trimmed = entry.trim()
        entry = trimmed + " -"
    }

    function pressEquals() {
        var trimmed = entry.trim()
        if (trimmed === "" || trimmed === "0") {
            backend.reportInputError("Пустой запрос: выражение не введено")
            return
        }
        backend.submitExpression(entry)
        resetAll()
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 10

        // ----- display -----
        Text {
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignRight
            font.pixelSize: 36
            text: entry
            elide: Text.ElideLeft
        }

        // ----- queue size indicators -----
        RowLayout {
            Layout.fillWidth: true
            Label { text: "Запросы в очереди: " + backend.requestQueueSize }
            Item { Layout.fillWidth: true }
            Label { text: "Результаты в очереди: " + backend.resultQueueSize }
        }

        // ----- engine / delay settings -----
        RowLayout {
            Layout.fillWidth: true
            Label { text: "Внешняя библиотека" }
            Switch {
                checked: backend.useExternalEngine
                enabled: backend.externalEngineAvailable
                onToggled: backend.useExternalEngine = checked
            }
            Item { Layout.fillWidth: true }
            Label { text: "Задержка, с:" }
            SpinBox {
                from: 0
                to: 30
                value: backend.calcDelaySeconds
                onValueModified: backend.calcDelaySeconds = value
            }
        }

        // ----- keypad -----
        GridLayout {
            Layout.fillWidth: true
            columns: 4
            rowSpacing: 6
            columnSpacing: 6

            Button { text: "7"; Layout.fillWidth: true; onClicked: appendDigit("7") }
            Button { text: "8"; Layout.fillWidth: true; onClicked: appendDigit("8") }
            Button { text: "9"; Layout.fillWidth: true; onClicked: appendDigit("9") }
            Button { text: "÷"; Layout.fillWidth: true; onClicked: pressOperator("/") }

            Button { text: "4"; Layout.fillWidth: true; onClicked: appendDigit("4") }
            Button { text: "5"; Layout.fillWidth: true; onClicked: appendDigit("5") }
            Button { text: "6"; Layout.fillWidth: true; onClicked: appendDigit("6") }
            Button { text: "×"; Layout.fillWidth: true; onClicked: pressOperator("*") }

            Button { text: "1"; Layout.fillWidth: true; onClicked: appendDigit("1") }
            Button { text: "2"; Layout.fillWidth: true; onClicked: appendDigit("2") }
            Button { text: "3"; Layout.fillWidth: true; onClicked: appendDigit("3") }
            Button { text: "−"; Layout.fillWidth: true; onClicked: pressOperator("-") }

            Button { text: "0"; Layout.fillWidth: true; onClicked: appendDigit("0") }
            Button { text: "."; Layout.fillWidth: true; onClicked: appendDot() }
            Button { text: "C"; Layout.fillWidth: true; onClicked: resetAll() }
            Button { text: "+"; Layout.fillWidth: true; onClicked: pressOperator("+") }

            Button {
                text: "="
                Layout.fillWidth: true
                Layout.columnSpan: 4
                onClicked: pressEquals()
            }
        }

        // ----- colored console -----
        Label { text: "Консоль" }
        ListView {
            id: consoleView
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            model: ListModel { id: consoleModel }
            delegate: Text {
                width: consoleView.width
                text: model.text
                color: model.color
                wrapMode: Text.Wrap
            }
            onCountChanged: positionViewAtEnd()

            Rectangle {
                anchors.fill: parent
                color: "#1e1e1e"
                z: -1
            }
        }
    }

    Connections {
        target: backend
        function onConsoleMessage(text, color) {
            consoleModel.append({ text: text, color: color })
        }
    }
}
