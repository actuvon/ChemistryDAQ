# Chemistry DAQ
A data aquisition system designed for use with analytical chemistry equipment. Log analog DC voltage signals from capilary eletrophoresis systems, atomic absorption systems, and chromatography equipment. 

### Files in this repo
- **Components.md**: List all the different parts used in the project, describe them, label the connection points, and indicate which diagrams to look at to understand how they are used.
- **./Components**: Files such as datasheets and other details relating to specific components.
- **Bill of Materials.md**: TODO: Provide purchasing information for all components.
- **User Manual.md**: TODO: Instructions on how to use and maintain the completed device.
- **./Images**: Photos or drawings, which are not diagrams.

### Understanding the "Components.md" file
- Each component is given a unique Component number that starts with the letter "C". For example, the amplifier module on channel two has the ID C1-2-2-1.
- Component numbers are given hierarchically. For example, the amplifier module C1-2-2-1 is a part of the channel two amplifier subassembly "C1-2-2", which is a part of the amplifier subasemblies section "C1-2", which is a part of the motherboard "C1".
- Components make connections with each other through "ports", which are identified with a collon. A port can be a mechanical **or** electrical connection point. In some cases a port can serve a mechanical and electrical function simultaniously, like how modules are mechanically mounted to the board by their electrical pin headers. As an example, the chip select (CS) pin on the ESP32 is given the ID C1-3:23. Component C1-3 is the ESP32, and we assigned the CS pin a port number of 23. We typically assign port numbers by going around a part in a counter-clockwise circle, as is done with many ICs.
- The "Connections" column can be used to identify two ports that are directly connected together. Sometimes this column is used to indicate components that are indirectly connected together as well. The intent is to provide intuition about the purpose of the component/port, and how it fits into the design. Multiple connections may be listed for a single component or port.
- The "Diagrams" column is used to specify a diagram number that shows how the parts works in a greater context. Multiple diagram numbers may be listed for a single component or port. For example, the Arduino Nano C1-1 may reference one 'electrical schematic' diagram that shows the electrical connections, and another 'layout diagram' that points out the physical locations of components on the motherboard.
- A URL may be provided for quick reference information, but this should not be considered the purchase link; that belongs in a file "Bill of Materials.md".
- Check the **./Components** folder for more detailed information such as datasheets, manuals, software, or implementation notes.