google.load("visualization", "1", {packages:["corechart"]});
google.setOnLoadCallback(drawChart);

function parseLineChart( str )
{
  rows = str.split( '\n' )
  var charts = []

  charts.push( rows[0].split( ',' ) )

 for( var i = 1; i < rows.length - 1; i++ ) {
    var row  = rows[i].split( ',' )
    var cols = []

    for( var j = 0; j < row.length; j++ ) {
       cols.push( parseInt( row[j] ) )
    }

    charts.push( cols )
 }

  return charts
}

function parseHistogram( str )
{
 var rows    = str.split( '\n' )
 var charts  = []

 charts.push( rows[0].split( ',' ) )

 for( var i = 1; i < rows.length - 1; i++ ) {
    var row  = rows[i].split( ',' )
    var cols = []

    for( var j = 0; j < row.length; j++ ) {
       cols.push( j == 0 ? row[j] : parseInt( row[j] ) )
    }

    charts.push( cols )
 }  

 return charts
}

function loadChart( type, title, fileName, target ) {
  var chart = null

  switch( type ) {
  case "hist": chart = new google.visualization.ColumnChart( document.getElementById( target ) ); break
  case "line": chart = new google.visualization.LineChart( document.getElementById( target ) );   break
  }

   $.get( fileName,
      function( csvString ) {
        var charts = null

        switch( type ) {
        case "hist": charts = parseHistogram( csvString ); break
        case "line": charts = parseLineChart( csvString ); break
        }

        chart.draw( google.visualization.arrayToDataTable( charts ), { legend: { position: "top" }, title: title, series: { 1: { curveType: 'function' }, 0: { curveType: 'function' }, 2: { curveType: 'function' } } } )
      } )
}