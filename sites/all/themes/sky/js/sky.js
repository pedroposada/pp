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
			var path = window.location.pathname;
			$('#block-system-main-menu li').each(function(i){
				var src = $('a', this).attr('href');
				if (src.length > 1) {
					src = src.replace(/\//g,"\\/");
					var pat = new RegExp('^' + src);
					console.log(pat);
					path.match(pat) && $(this).addClass('active-trail').find('a').addClass('active-trail active');
				};
			});
			// path.match(/^\/portfolio\//) && $('li.menu-item-1685').addClass('active-trail').find('a').addClass('active-trail active');
			// path.match(/^\/blog\//) && $('li.menu-item-719').addClass('active-trail').find('a').addClass('active-trail active');
			// path.match(/^\/category\/blog\//) && $('li.menu-item-719').addClass('active-trail').find('a').addClass('active-trail active');
			// path.match(/^\/gallery\//) && $('li.menu-item-1928').addClass('active-trail').find('a').addClass('active-trail active');
		}
	};
})(jQuery);
