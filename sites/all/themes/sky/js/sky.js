(function($) {
	Drupal.behaviors.pp = {
  	attach: function (context, settings) {

			// add watermark to the search box
			$('input[name="search_block_form"]').watermark('search');

			// add spans to summary lists items counts
			$('.display-summary-list-style ul.views-summary li').each(function(i){
        var src = $(this).html();
        var pat = /([\s\S]*)\((\d+)\)/i;
        var m = src.match(pat);
        var newSrc = m[1] + '<span class="items-count">' + m[2] + '</span>';
        $(this).html(newSrc);
			});
			
			// make proper tab active in node page
			window.location.pathname.match(/^\/portfolio\//) && $('li.menu-item-1685').addClass('active-trail').find('a').addClass('active-trail active');
			window.location.pathname.match(/^\/blog\//) && $('li.menu-item-719').addClass('active-trail').find('a').addClass('active-trail active');
		}
	};
})(jQuery);
