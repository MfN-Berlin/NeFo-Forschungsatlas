# NeFo-Forschungsatlas
NeFo-Forschungsatlas module

# Dependencies
* Drupal 7 [Chosen module](https://www.drupal.org/project/chosen)
* Drupal 7 [Geocoder module](https://www.drupal.org/project/geocoder)
* Drupal 7 [Libraries API module](https://www.drupal.org/project/libraries)
## Leaflet Library
* Download [Leaflet v0.7.7](http://leafletjs.com/download.html) and put it in your `sites/all/libraries` folder.
* Download [Leaflet Markercluster v0.1](https://github.com/leaflet/Leaflet.markercluster/downloads) and put it in your `sites/all/libraries` folder.

# Installation
1. Install and enable the NeFo-Forschungsatlas module and its required modules Chosen and Geocoder in the usual way.
2. Execute the MySQL script forschungsatlas.install.sql (CREATE FUNCTION and CREATE VIEW privileges are required).

# Uninstall
1. Uninstall the NeFo-Forschungsatlas module in the usual way.
2. Execute the MySQL script forschungsatlas.uninstall.sql (DROP FUNCTION and DROP VIEW privileges are required).
