(function ($) {
  Drupal.behaviors.forschungsatlas = {
    attach: function (context, settings) {
      jQuery(settings.forschungsatlas).each(function() {
        // load a settings object with all of our map settings
        var settings = {};
        for (var setting in this.map.settings) {
          settings[setting] = this.map.settings[setting];
        }

        // instantiate our new map
        var lMap = new L.Map(this.mapId, settings);
        lMap.setView([51,11], 5);
        lMap.setZoom(6);
        //lMap.bounds = [];
        // FIXME aus Settings laden
        var cloudmade = L.tileLayer('http://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png', {
          maxZoom: 18,
          attribution: 'Map data &copy; 2011 OpenStreetMap contributors',
        });
        lMap.addLayer(cloudmade);

        var createClusterMarker = function (clusterCount, pos) {
          // TESTING custom html icon
          var myIcon = L.divIcon({
            className: 'cluster',
            html: "<div class='cluster--inner'>" + clusterCount + "</div>",
            iconSize: null,
            iconAnchor: [17,17]
          });
          // you can set .my-div-icon styles in CSS
          return L.marker(pos, {icon: myIcon});;
        };


        var model = {
          states: {},
          statenames: {}
        };

        //console.log(this.features);

        for (var i in this.features) {
          var feature = this.features[i];
          stateId = feature.fsid;
          if (!model.states.hasOwnProperty(stateId)) {
            model.states[stateId] = {
              features: []
            };
          }
          // FIXME. Data Mismatch between geojson file and features supplied by Drupal
          if (feature.fsname == "Freie Hansestadt Bremen") {
            feature.fsname = "Bremen";
          }
          if (!model.statenames.hasOwnProperty(feature.fsname)) {
            model.statenames[feature.fsname] = stateId;
          }
          model.states[stateId].features.push(feature);
        }

        // Create federal state overlay
        var defaultStyle = {
          color: "#2262CC",
          weight: 1,
          opacity: 0.6,
          fillOpacity: 0.1,
          fillColor: "#5595FF"
        };

        // FIXME
        var geojsonUrl = this.geojsonUrl;
        var geolayer = new L.featureGroup();
        //console.log("test");
        //jquery.getJSON would throw syntax error due to wrong contentType information
        jQuery.getJSON(geojsonUrl, function (json) {
          L.geoJson(json, {
            onEachFeature: function (feature, layer) {
              //Name des Bundeslandes
              var statename = layer.feature.properties.GEN;

              //sammle alle Bundesland-Layer in LayerGroup geolayer
              geolayer.addLayer(layer);

              //Darstellung beginnt mit abgeschwaecher Deckung fuer Bundesland (Default-Style)
              layer.setStyle(defaultStyle);
              var stateId = model.statenames[statename];
              if (stateId) {
                var centerPos = layer.getBounds().getCenter();
                model.states[stateId].pos = centerPos;
              }
              // click on state overlay
              layer.on('click', function (e) {
                lMap.removeLayer(model.states[stateId].layer);
                model.states[stateId].cluster.addTo(lMap);
                lMap.fitBounds(e.target.getBounds(), {animate: true, duration: 1});
              });

            }
          }).addTo(lMap);

          for (var s in model.states) {
            var state = model.states[s];
            //console.log(state);
            var stateLayer = L.markerClusterGroup({
              showCoverageOnHover: false
            });
            var markers = state.features.map(function (feature) {
              var m = L.marker([feature.lat, feature.lon]);
              m.bindPopup(feature.popup);
              return m;
            });

            for (var m in markers) {
              var marker = markers[m];
              stateLayer.addLayer(marker);
            }
            state.layer = stateLayer;

            // function for scoping state and statelayer
            (function(sl, state) {

              //console.log(model, state);
              var marker = createClusterMarker(state.features.length, state.pos);
              //click on state cluster marker
              marker.on('click', function (e) {
                lMap.removeLayer(e.target);
                sl.addTo(lMap);
                lMap.fitBounds(sl.getBounds(), {animate: true, duration: 1});
              });
              marker.addTo(lMap);
              state.cluster = marker;
            })(stateLayer, state);
          }
        });
      });
    }
  };
})(jQuery);
