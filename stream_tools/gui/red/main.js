/** Modified from original Node-Red source, for audio system visualization
 * vim: set ts=4:
 * Copyright 2013 IBM Corp.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 **/
var RED = (function() {

	$('#btn-keyboard-shortcuts').click(function(){showHelp();});

	function hideDropTarget() {
		$("#dropTarget").hide();
		RED.keyboard.remove(/* ESCAPE */ 27);
	}

	$('#chart').on("dragenter",function(event) {
		if ($.inArray("text/plain",event.originalEvent.dataTransfer.types) != -1) {
			$("#dropTarget").css({display:'table'});
			RED.keyboard.add(/* ESCAPE */ 27,hideDropTarget);
		}
	});

	$('#dropTarget').on("dragover",function(event) {
		if ($.inArray("text/plain",event.originalEvent.dataTransfer.types) != -1) {
			event.preventDefault();
		}
	})
	.on("dragleave",function(event) {
		hideDropTarget();
	})
	.on("drop",function(event) {
		var data = event.originalEvent.dataTransfer.getData("text/plain");
		hideDropTarget();
		RED.view.importNodes(data);
		event.preventDefault();
	});
	function make_name(n) {
		var name = (n.name ? n.name : n.id);
		name = name.replace(" ", "_").replace("+", "_").replace("-", "_");
		return name
	}

    function isEmpty(str) {
        return (!str || str.length === 0);
    }
	function save(force) {
		RED.storage.update();
		var export_as_yaml = 1;

		if (RED.nodes.hasIO()) 
        {
			var nns = RED.nodes.createCompleteNodeSet();
			// sort by horizontal position, plus slight vertical position,
			// for well defined update order that follows signal flow
			nns.sort(function(a,b){ return (a.x + a.y/250) - (b.x + b.y/250); });
			// console.log(JSON.stringify(nns));

			// TODO Investigate why graph tool malfunctions when this compatability feature is removed
			var metaData = $.parseJSON($("script[data-container-name|='InputOutputCompatibilityMetadata']").html());
			requirements = metaData["requirements"];


			// Create empty string which will be the YML format graph 
			// Note YML does not use TABS, always use spaces
			var yml = "";
			if (export_as_yaml){
				yml += "version: " + "'1.0'" + "\n";
				// yml += "graph:" + "\n";
				yml += "  nodes:" + "\n";
			}

			// generate code for all data processing nodes
			for (var i=0; i<nns.length; i++) {
				var n = nns[i];
				var node = RED.nodes.node(n.id);

				if (node && (node.outputs > 0 || node._def.inputs > 0)) {
					var name = make_name(n)
					// TODO When graph is finalised create an array of field types to iterate through
					// or at least remove the if (!undefined) logic for fields that are universal
					if (export_as_yaml){
						//yml += "  - node: " + name + "\n";
						//yml += "    kind: " + node["kind"] + "\n";
						//if (node["samplingRate"] != undefined)  {yml += "    samplingRate: " + String(node["samplingRate"]) + "\n"}
						//if (node["NbChan"] != undefined)        {yml += "    NbChan: " + String(node["NbChan"]) + "\n"}
						//if (node["interleaving"] != undefined)  {yml += "    interleaving: " + String(node["interleaving"]) + "\n"}
                        //if (node["timeStamp"] != undefined)     {yml += "    timeStamp: " + String(node["timeStamp"]) + "\n" }

                        if (node["kind"] == "IO") {
                            yml += "  - IO:   " + name + "\n";
                            if (! isEmpty(node["FRAMEL"])) { yml += "    FRAMEL: " + String(node["FRAMEL"]) + "\n" } // frame length in samples
                            //if (node["FRAMEL"] != undefined) { yml += "    FRAMEL: " + String(node["FRAMEL"]) + "\n" } // frame length in samples
                            if (! isEmpty(node["NBCHAN"])) { yml += "    NBCHAN: " + String(node["NBCHAN"]) + "\n" } // number of channel options 
                        } 
                        if (node["kind"] == "node") {
                            yml += "  - node: " + name + "\n";
                            if (! isEmpty(node["PRESET"])) { yml += "    PRESET: " + String(node["PRESET"]) + "\n" }
                            if (! isEmpty(node["SCRIPT"])) { yml += "    SCRIPT: " + String(node["SCRIPT"]) + "\n" }
                        }
						//if (node._def.inputs > 0) {
						//	yml += "    inputs:\n";
						//	for (var k=0; k< node._def.inputs; k++) {
						//		yml += "    - input: IPort" + String(k) + "\n";
						//		if (node["samples"] != undefined) {yml += "      samples: " + String(node["samples"]) + "\n"}
						//		//yml += "      type: " + String(node["input_node_dataformats"]) + "\n";
						//	}
						//}
						//if (node.outputs > 0) {
						//	yml += "    outputs:\n";
						//	for (var k=0; k< node.outputs; k++) {
						//		yml += "    - output: OPort" + String(k) + "\n";
						//		if (node["samples"] != undefined) {yml += "      samples: " + String(node["samples"]) + "\n"}
						//		//yml += "      type: " + String(node["output_node_dataformats"]) + "\n";
						//	}
						//}
						// TODO Confirm this approach is robust as JSON object indices may not be reliable
						// Abandoned more elegant way of finding index using  var arg_starting_index = node.findIndex("numArgs");
						// Find index of numArgs+1 to access the individual arguments by index [numArgs+offset] rather than name
						var index = 0;
						for (const key in node) {
							if (key =="numArgs"){
								index++;
								break;
							}
							index++;
						}
						if(node.numArgs > 0){
							yml += "    args:\n"
							for (var l=0; l< node.numArgs; l++) {
								yml += "    - " + Object.keys(node)[index+l] + ": " + Object.values(node)[index+l] + "\n";
							}
						}
					}

					else { // if !export_as_yaml
						// Original JSON output format that can be imported.
						// TODO  Investigate loop parameters type.length/n.id.length as they seem arbitrary
						yml += n.type + " ";
						for (var j=n.type.length; j<24; j++) yml += " ";
						yml += name + "; ";
						for (var j=n.id.length; j<14; j++) yml += " ";
						// yml += "//xy=" + Math.round(n.x) + "," + Math.round(n.y);
					}
                }
            }
        
            // generate code for all connections (aka wires or links)
            var cordcount = 1; // TODO remove if un-used
            yml += "  arcs:\n";
            for (var i=0; i<nns.length; i++) {
                var n = nns[i];
                // TODO Add if n.args?
                if (n.wires) {
                    for (var j=0; j<n.wires.length; j++) {
                        var wires = n.wires[j];
                        if (!wires) continue;
                        for (var k=0; k<wires.length; k++) {
                            var wire = n.wires[j][k];
                            if (wire) {
                                var parts = wire.split(":");
                                // TODO Investigate when/why parts.length is not == 2
                                // if (parts.length == 2) {
                                var src = RED.nodes.node(n.id);
                                var dst = RED.nodes.node(parts[0]);
                                var src_name = make_name(src);
                                var dst_name = make_name(dst);
                                if (parts.length > 0) {
                                    // Note: Removed this line as logic was unclear
                                    // if (j == 0 && parts[1] == 0 && src && src.outputs == 1 && dst && dst._def.inputs == 1) {
                                    if (src && dst) {
                                        if (export_as_yaml) {
                                            if (src["kind"] == "IO") {
                                                yml += "  - OPort" + String(j) + " IO:   " + src_name + "\n";
                                            } else {
                                                yml += "  - OPort" + String(j) + " node: " + src_name + "\n";
                                            }

                                            if (dst["kind"] == "IO") {
                                                yml += "    IPort" + String(parts[1]) + " IO:   " + dst_name + "\n";
                                            } else {                                 
                                                yml += "    IPort" + String(parts[1]) + " node: " + dst_name + "\n";
                                            } 
                                        } else {
                                            yml += "Arc" + "    Arc" + cordcount + "(";
                                            yml += src_name + ", " + j + ", " + dst_name + ", " + parts[1];
                                        yml += ");\n";
                                        }
                                    }
                            cordcount++;
                                }
                            }
                        }
                    }
                }
            }
		
            // generate code for all control nodes (no inputs or outputs)
            for (var i=0; i<nns.length; i++) {
                var n = nns[i];
                var node = RED.nodes.node(n.id);
                if (node && node.outputs == 0 && node._def.inputs == 0) 
                {
                    n1 = make_name(n);
                    if (n1 == "script") { yml +=  n.type + " "; } 
                    else { yml += "//" + n.type + " "; }
                    
                    for (var j=n.type.length; j<24; j++) yml += " ";
                    yml += make_name(n) + "\n";
                    for (var j=n.id.length; j<14; j++) yml += " ";
                    // yml += "//xy=" + Math.round(n.x) + "," + Math.round(n.y) + "\n";
                }
            }

            // console.log(yml);

			RED.view.state(RED.state.EXPORT);
			RED.view.getForm('dialog-form', 'export-clipboard-dialog', function (d, f) {
				$("#node-input-export").val(yml).focus(function() {
				var textarea = $(this);
				textarea.select();
				textarea.mouseup(function() {
					textarea.unbind("mouseup");
					return false;
				});
				}).focus();
			$( "#dialog" ).dialog("option","title","Export Graph").dialog( "open" );
			});
			//RED.view.dirty(false);
		} else 
        {
			$( "#node-dialog-error-deploy" ).dialog({
				title: "Error exporting data to CMSIS",
				modal: true,
				autoOpen: false,
				width: 410,
				height: 245,
				buttons: [{
					text: "Ok",
					click: function() {
						$( this ).dialog( "close" );
					}
				}]
			}).dialog("open");
		}
	}

	$('#btn-deploy').click(function() { save(); });


	$( "#node-dialog-confirm-deploy" ).dialog({
			title: "Confirm deploy",
			modal: true,
			autoOpen: false,
			width: 530,
			height: 230,
			buttons: [
				{
					text: "Confirm deploy",
					click: function() {
						save(true);
						$( this ).dialog( "close" );
					}
				},
				{
					text: "Cancel",
					click: function() {
						$( this ).dialog( "close" );
					}
				}
			]
	});

	// from http://css-tricks.com/snippets/javascript/get-url-variables/
	function getQueryVariable(variable) {
		var query = window.location.search.substring(1);
		var vars = query.split("&");
		for (var i=0;i<vars.length;i++) {
			var pair = vars[i].split("=");
			if(pair[0] == variable){return pair[1];}
		}
		return(false);
	}
    // function getQueryVariable(variable) {
    //   var query = window.location.search.substring(1)
    //   var vars = query.split("&");
    //   for (var i=0;i<vars.length;i++) {
    //       var pair = vars[i].split("=")
    //       if(pair[0] == variable){
    //           if(pair[1].indexOf('%20') != -1){
    //               console.log(pair[1].indexOf('%20'))
    //               var fullName = pair[1].split('%20')
    //               console.log(fullName)
    //               return fullName[0] + ' ' + fullName[1]
    //           }
    //           else {
    //               return pair[1];
    //           }
    //       }
    //   }
    //   return(false)    
    // }

	function loadNodes() {
			$(".palette-scroll").show();
			$("#palette-search").show();
			RED.storage.load();
			RED.view.redraw();
			setTimeout(function() {
				$("#btn-deploy").removeClass("disabled").addClass("btn-danger");
				$("#btn-import").removeClass("disabled").addClass("btn-success");
			}, 1500);
			$('#btn-deploy').click(function() { save(); });
			// if the query string has ?info=className, populate info tab
			var info = getQueryVariable("info");
			if (info) {
				RED.sidebar.info.setHelpContent('', info);
			}
	}

	$('#btn-node-status').click(function() {toggleStatus();});

	var statusEnabled = false;
	function toggleStatus() {
		var btnStatus = $("#btn-node-status");
		statusEnabled = btnStatus.toggleClass("active").hasClass("active");
		RED.view.status(statusEnabled);
	}
	
	function showHelp() {

		var dialog = $('#node-help');

		//$("#node-help").draggable({
		//        handle: ".modal-header"
		//});

		dialog.on('show',function() {
			RED.keyboard.disable();
		});
		dialog.on('hidden',function() {
			RED.keyboard.enable();
		});

		dialog.modal();
	}

	$(function() {
		$(".palette-spinner").show();

		// server test switched off - test purposes only
		var patt = new RegExp(/^[http|https]/);
		var server = false && patt.test(location.protocol);

		if (!server) {
			// TODO Investigate why graph tool malfunctions when this compaability feature is removed
			var metaData = $.parseJSON($("script[data-container-name|='InputOutputCompatibilityMetadata']").html());
			requirements = metaData["requirements"];
			
			var data = $.parseJSON($("script[data-container-name|='NodeDefinitions']").html());
			var nodes = data["nodes"];
			$.each(nodes, function (key, val) {
				RED.nodes.registerType(val["type"], val["data"]);
			});
			RED.keyboard.add(/* ? */ 191, {shift: true}, function () {
				showHelp();
				d3.event.preventDefault();
			});
			loadNodes();
			$(".palette-spinner").hide();
		} else {
			$.ajaxSetup({beforeSend: function(xhr){
				if (xhr.overrideMimeType) {
					xhr.overrideMimeType("application/json");
				}
			}});
			$.getJSON( "resources/nodes_def.json", function( data ) {
				var nodes = data["nodes"];
				$.each(nodes, function(key, val) {
					RED.nodes.registerType(val["type"], val["data"]);
				});
				RED.keyboard.add(/* ? */ 191,{shift:true},function(){showHelp();d3.event.preventDefault();});
				loadNodes();
				$(".palette-spinner").hide();
			})
		}
	});

	return {
	};
})();
