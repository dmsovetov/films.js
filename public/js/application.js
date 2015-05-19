$(function() {
    // ** itemRenderer
    function itemRenderer( data ){
        return  '<div style="padding: 5px; overflow:hidden;">' +
                '<div style="float: left; margin-left: 5px">' +
                '<div style="font-weight: bold; color: #333; font-size: 15px; line-height: 11px">' + data.name + ' (' + data.year + ')' + '</div>' +
                '<div style="color: #999; font-size: 14px">' + data.genres + '</div>' +
                '</div>' +
                '</div><div style="clear:both;"></div>'; // make sure we have closed our dom stuff
            }

    var ms = $( '#query' ).magicSuggest( {
        allowFreeEntries:   false,
        allowDuplicates:    false,
        useCommaKey:        false,
        method:             'get',
        data:               'items',
        placeholder:        'Введите одно или несколько названий фильмов, которые Вам понравились.',
        required:           true,
        maxSuggestions:     10,
        minChars:           3,
        renderer:           itemRenderer
    } )

    $(ms).on( 'selectionchange',
        function( e, m ) {
            $("#progress").html( "<br/> <img src='images/loader.gif' />" )
            $("#content").html( "" )
            ms.disable()
            $.get( "similar?query=" + JSON.stringify( this.getValue() ),
                function( data ) {
                    ms.enable()
                    $("#progress").html( "" )
                    $("#content").html( data )
                } )
        } )
} )