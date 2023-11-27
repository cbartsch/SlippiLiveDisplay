import Felgo 4.0
import QtQuick 2.0

import SlippiLive

AppPage {
  title: "Settings"

  Column {
    id: contentCol
    width: parent.width

    SimpleSection {
      title: "Overlays"
    }

    CheckableListItem {
      text: "Show Combos"

      checked: settings.showComboOverlay
      onCheckedChanged: settings.showComboOverlay = this.checked
    }

    CheckableListItem {
      text: "Show L-Cancels"

      checked: settings.showLCancelOverlay
      onCheckedChanged: settings.showLCancelOverlay = this.checked
    }

    CheckableListItem {
      text: "Show Wavedashes"

      checked: settings.showWavedashOverlay
      onCheckedChanged: settings.showWavedashOverlay = this.checked
    }

    CheckableListItem {
      text: "Show Fastfalls"

      checked: settings.showFastfallOverlay
      onCheckedChanged: settings.showFastfallOverlay = this.checked
    }
  }

  component CheckableListItem : AppListItem {
    property alias checked: checkBox.checked

    onSelected: checkBox.toggle()

    leftItem: AppCheckBox {
      id: checkBox
      anchors.verticalCenter: parent.verticalCenter
    }
  }
}
