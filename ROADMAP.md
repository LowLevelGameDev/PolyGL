# Developer Roadmap

## Add Better Error Checking!!!!!!
Currently there is not much checking if anything goes wrong with an internal vulkan function. <br>
Mallocs are checked for the most part but some may be missed. <br>
Remember to add to the err map and consider making it public. <br>

## Device Selection
Currently device selection is hidden, <br>
realistically there should be more control given to the user in the device selected. <br>

## add optional implementation headers for structs
In case of projects trying to integrate this (rather than include) more direct control may be needed, <br>
this should be fully supported. <br>

## static library
it doesn't make much sense to have a dynamic graphics library in my opinion, <br>
if this was to be handled by a game manager like steam for all games that use it then maybe, <br>
but currently it will just be installed with the game and then dynamically linked. <br>
Copies will still exist for each game packaged with it making any advantages of dynamic <br> libraries kind of useless. <br>
<br>
Either way support for both static and dynamic versions should exist. <br>
