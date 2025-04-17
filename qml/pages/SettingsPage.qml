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
      detailText: "Displays in-game combo counter"

      checked: settings.showComboOverlay
      onCheckedChanged: settings.showComboOverlay = this.checked
    }

    CheckableListItem {
      text: "Show L-Cancels"
      detailText: "Displays L-cancel timing and success"

      checked: settings.showLCancelOverlay
      onCheckedChanged: settings.showLCancelOverlay = this.checked
    }

    CheckableListItem {
      text: "Show Wavedashes"
      detailText: "Displays wavedash angle and timing"

      checked: settings.showWavedashOverlay
      onCheckedChanged: settings.showWavedashOverlay = this.checked
    }

    CheckableListItem {
      text: "Show Fastfalls"
      detailText: "Displays fastfall timing"

      checked: settings.showFastfallOverlay
      onCheckedChanged: settings.showFastfallOverlay = this.checked
    }

    CheckableListItem {
      text: "Show Character-Specific stats"
      detailText: "Displays: Luigi cyclone mash counter (other suggestions welcome)"

      checked: settings.showCharSpecificOverlay
      onCheckedChanged: settings.showCharSpecificOverlay = this.checked
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
