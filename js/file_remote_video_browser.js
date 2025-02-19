(function ($, Backdrop) {

  Backdrop.behaviors.browserLink = {
    attach: function(context, settings) {
      $('.file-remote-video-browser').each(function(){
        let browser = $(this);
        let fidInput = browser.find('input[name="fid"]').eq(0);
        let opInput = browser.find('input[name="frv_op"]').eq(0);
        let links = $('.view-file-remote-video-library .file-remote-video-browser-views-link');
        links.once().on('click', function(e) {
          e.preventDefault();
          fidInput.val('');
          links.filter('.is-selected').removeClass('is-selected');
          $(this).addClass('is-selected');
          fidInput.val($(this).data('fid'));
          return false;
        });
        let tabs = browser.find('.tabs');
        let tabItems = tabs.find('li');
        let tabLinks = tabItems.find('a');
        tabItems.find('a').once().on('click', function(e) {
          e.preventDefault();
          if (!$(this).parent('li').hasClass('active')) {
            tabItems.filter('.active').removeClass('active');
            $(this).parent('li').addClass('active');
            tabLinks.each(function() {
              if ($(this).parent('li').hasClass('active')) {
                $($(this).data('selector')).show();
              }
              else {
                $($(this).data('selector')).hide();
              }
            });
            opInput.val($(this).data('op'));
          }
          return false;
        }).eq(0).trigger('click');
      });
    }
  };

  Backdrop.fileRemoteVideo = {
    dialogCloseEvent: function(e, dialog, element) {
      if (element.attr('id') == 'file-remote-video-browser-modal')  {
        const browser = Backdrop.settings.fileRemoteVideoBrowser;
        let fidElement = $(`input[name="${browser.fidElement}"]`);
        fidElement.val(browser.selectedFid);
        $(`input[name="${browser.refreshButton}"]`).trigger('mousedown').trigger('mouseup').trigger('click');
      }
    }
  };

  $(window).on('dialog:afterclose',  Backdrop.fileRemoteVideo.dialogCloseEvent);
})(jQuery, Backdrop);
