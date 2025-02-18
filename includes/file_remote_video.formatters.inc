<?php

/**
 * @file
 * File formatters for YouTube videos.
 */

/**
 * Implements hook_field_formatter_info().
 */
function file_remote_video_field_formatter_info() {
  $config = config('file_remote_video.settings');
  $settings = $config->getData();
  $settings['youtube_player'] = 'global';
  $settings['vimeo_player'] = 'global';
  $formatters['file_remote_video'] = [
    'label' => t('Remote Video'),
    'field types' => ['file'],
    'settings' => $settings,
    'mime types' => ['video/youtube', 'video/vimeo'],
  ];

  return $formatters;
}

/**
 * Implements hook_field_formatter_settings_form().
 */
function file_remote_video_field_formatter_settings_form($field, $instance, $view_mode, $form, &$form_state) {
  $display = $instance['display'][$view_mode];
  $settings = $display['settings'];
  if ($display['type'] == 'file_remote_video') {
    $element = [];
    $element += file_remote_video_field_formatter_settings_form_youtube($field, $instance, $view_mode, $form, $form_state);
    $element += file_remote_video_field_formatter_settings_form_vimeo($field, $instance, $view_mode, $form, $form_state);
    return $element;
  }
}

function file_remote_video_field_formatter_settings_form_youtube($field, $instance, $view_mode, $form, &$form_state) {
  $display = $instance['display'][$view_mode];
  $settings = $display['settings']['youtube'];
  $field_name = $field['field_name'];

  $config_link = l(t('remote video player settings'), 'admin/config/media/file-remote-video-settings');
  $element = [
    'youtube_player' => [
      '#type' => 'select',
      '#title' => t('Youtube Player Settings'),
      '#description' => t('The global settings can be found at: !link ', [
        '!link' => $config_link
      ]),
      '#options' => [
        'global' => t('Use the global player'),
        'custom' => t('Provide custom settings for this instance'),
      ],
      '#default_value' => $display['settings']['youtube_player'],
      '#ajax' => [
        'callback' => 'file_remote_video_field_formatter_settings_form_youtube_ajax',
        'wrapper' => 'file-remote-video-youtube-wrapper',
      ]
    ],
    'youtube' => [
      '#type' => 'markup',
      '#markup' => '<div id="file-remote-video-youtube-wrapper"></div>',
    ]
  ];
  $show_custom_settings = $display['settings']['youtube_player'] == 'custom';
  $form_submitted = isset($form_state['values']['fields'][$field_name]['settings_edit_form']);
  if ($form_submitted) {
    $show_custom_settings = $form_state['values']['fields'][$field_name]['settings_edit_form']['settings']['youtube_player'] == 'custom';
  }
  if ($show_custom_settings) {
    $fieldset = file_remote_video_field_formatter_settings_form_youtube_fieldset($settings);
    $element['youtube'] = $fieldset['youtube'];
  }
  return $element;
}

function file_remote_video_field_formatter_settings_form_youtube_ajax($form, $form_state) {
  $field_name = $form_state['formatter_settings_edit'];
  return $form['fields'][$field_name]['format']['settings_edit_form']['settings']['youtube'];
}

function file_remote_video_field_formatter_settings_form_youtube_fieldset($settings) {
  $element['youtube'] = [
    '#type' => 'fieldset',
    '#title' => t('YouTube Settings'),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
    '#tree' => TRUE,
    '#prefix' => '<div id="file-remote-video-youtube-wrapper">',
    '#suffix' => '</div>',
  ];
  $element['youtube']['width'] = array(
    '#title' => t('Width'),
    '#type' => 'textfield',
    '#default_value' => $settings['width'],
    '#element_validate' => array('file_remote_video_youtube_validate_video_width_and_height'),
  );
  $element['youtube']['height'] = array(
    '#title' => t('Height'),
    '#type' => 'textfield',
    '#default_value' => $settings['height'],
    '#element_validate' => array('file_remote_video_youtube_validate_video_width_and_height'),
  );

  // @see https://developers.google.com/youtube/player_parameters#Parameters

  // Multiple options.
  $element['youtube']['theme'] = array(
    '#title' => t('Player theme'),
    '#type' => 'radios',
    '#options' => array(
      'dark' => t('Dark'),
      'light' => t('Light'),
    ),
    '#default_value' => $settings['theme'],
  );
  $element['youtube']['color'] = array(
    '#title' => t('Progress bar color'),
    '#type' => 'radios',
    '#options' => array(
      'red' => t('Red'),
      'white' => t('White'),
    ),
    '#default_value' => $settings['color'],
  );
  $element['youtube']['autohide'] = array(
    '#title' => t('Controls during playback'),
    '#type' => 'radios',
    '#options' => array(
      '0' => t('Keep progress bar and player controls on screen while playing'),
      '2' => t('Hide progress bar while playing'),
      '1' => t('Hide progress bar and player controls'),
    ),
    '#default_value' => $settings['autohide'],
  );
  $element['youtube']['captions'] = array(
    '#title' => t('Captions displaying options'),
    '#type' => 'radios',
    '#options' => array(
      '0' => t('Turns captions off by default'),
      '1' => t('Turns captions on by default'),
      '2' => t('Turns captions on and set the caption language for the video by default'),
    ),
    '#default_value' => $settings['captions'],
  );

  // Single Options.
  $element['youtube']['autoplay'] = array(
    '#title' => t('Autoplay video on load'),
    '#type' => 'checkbox',
    '#default_value' => $settings['autoplay'],
  );
  $element['youtube']['loop'] = array(
    '#title' => t('Loop video'),
    '#type' => 'checkbox',
    '#default_value' => $settings['loop'],
  );
  $element['youtube']['controls'] = array(
    '#title' => t('Show Controls'),
    '#type' => 'checkbox',
    '#default_value' => $settings['controls'],
  );

  // Note: make sure the positive/negative language lines up with option
  // processing in file_remote_video.theme.inc.
  $element['youtube']['showinfo'] = array(
    '#title' => t('Display video title and uploader'),
    '#type' => 'checkbox',
    '#default_value' => $settings['showinfo'],
  );
  $element['youtube']['modestbranding'] = array(
    '#title' => t('Remove YouTube logo from the control bar'),
    '#type' => 'checkbox',
    '#default_value' => $settings['modestbranding'],
  );
  $element['youtube']['rel'] = array(
    '#title' => t('Show related videos when playback ends'),
    '#type' => 'checkbox',
    '#default_value' => $settings['rel'],
  );
  $element['youtube']['nocookie'] = array(
    '#title' => t('Use privacy enhanced (no cookie) mode'),
    '#type' => 'checkbox',
    '#default_value' => $settings['nocookie'],
  );
  $element['youtube']['protocol_specify'] = array(
    '#title' => t('Specify an http protocol'),
    '#type' => 'checkbox',
    '#default_value' => $settings['protocol_specify'],
    '#description' => t('An explicit protocol may be necessary for videos embedded in RSS feeds and emails. If no protocol is specified, iframes will be protocol relative.'),
  );
  $element['youtube']['protocol'] = array(
    '#title' => t('Iframe protocol'),
    '#type' => 'radios',
    '#default_value' => $settings['protocol'],
    '#options' => array(
      'https:' => 'https://',
    ),
    '#states' => array(
      'invisible' => array(
        ':input[name="displays[file_remote_video_video][settings][youtube][protocol_specify]"]' => array('checked' => FALSE),
      ),
    ),
  );

  // JS api.
  $element['youtube']['enablejsapi'] = array(
    '#title' => t('Enable the <a href="https://developers.google.com/youtube/js_api_reference">Javascript API</a>'),
    '#type' => 'checkbox',
    '#default_value' => $settings['enablejsapi'],
    '#description' => 'An id in the format <code>media-youtube-{video_id}</code> will be added to each iframe.',
  );
  $element['youtube']['origin'] = array(
    '#title' => t('Origin'),
    '#type' => 'textfield',
    '#description' => t('If the Javascript API is enabled, enter your site\'s domain for added security.'),
    '#default_value' => $settings['origin'],
    '#states' => array(
      'invisible' => array(
        ':input[name="displays[file_remote_video_video][settings][youtube][enablejsapi]"]' => array('checked' => FALSE),
      ),
    ),
    '#element_validate' => array('file_remote_video_youtube_validate_jsapi_domain'),
  );
  return $element;
}

function file_remote_video_field_formatter_settings_form_vimeo($field, $instance, $view_mode, $form, &$form_state) {
  $display = $instance['display'][$view_mode];
  $settings = $display['settings']['vimeo'];
  $field_name = $field['field_name'];
  $config_link = l(t('remote video player settings'), 'admin/config/media/file-remote-video-settings');
  $element = [
    'vimeo_player' => [
      '#type' => 'select',
      '#title' => t('Vimeo Player Settings'),
      '#description' => t('The global settings can be found at: !link ', [
        '!link' => $config_link
      ]),
      '#options' => [
        'global' => t('Use the global player'),
        'custom' => t('Provide custom settings for this instance'),
      ],
      '#default_value' => $display['settings']['vimeo_player'],
      '#ajax' => [
        'callback' => 'file_remote_video_field_formatter_settings_form_vimeo_ajax',
        'wrapper' => 'file-remote-video-vimeo-wrapper',
      ]
    ],
    'vimeo' => [
      '#type' => 'markup',
      '#markup' => '<div id="file-remote-video-vimeo-wrapper"></div>',
    ]
  ];
  $show_custom_settings = $display['settings']['vimeo_player'] == 'custom';
  $form_submitted = isset($form_state['values']['fields'][$field_name]['settings_edit_form']);
  if ($form_submitted) {
    $show_custom_settings = $form_state['values']['fields'][$field_name]['settings_edit_form']['settings']['vimeo_player'] == 'custom';
  }
  if ($show_custom_settings) {
    $fieldset = file_remote_video_field_formatter_settings_form_vimeo_fieldset($settings);
    $element['vimeo'] = $fieldset['vimeo'];
  }
  return $element;
}

function file_remote_video_field_formatter_settings_form_vimeo_fieldset($settings) {
  $element['vimeo'] = [
    '#type' => 'fieldset',
    '#title' => t('Vimeo Settings'),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
    '#prefix' => '<div id="file-remote-video-vimeo-wrapper">',
    '#suffix' => '</div>',
    '#tree' => TRUE,
  ];
  $element['#title'] = t('Vimeo Settings');
  $element['vimeo']['width'] = array(
    '#title' => t('Width'),
    '#type' => 'textfield',
    '#default_value' => $settings['width'],
    '#element_validate' => array('file_remote_video_vimeo_validate_video_width_and_height'),
  );
  $element['vimeo']['height'] = array(
    '#title' => t('Height'),
    '#type' => 'textfield',
    '#default_value' => $settings['height'],
    '#element_validate' => array('file_remote_video_vimeo_validate_video_width_and_height'),
  );

  // @see http://developer.vimeo.com/player/embedding

  // Multiple and string options.
  $element['vimeo']['color'] = array(
    '#title' => t('Custom video controls color'),
    '#type' => 'textfield',
    '#default_value' => $settings['color'],
    '#description' => t('A 3 or 6 character hex color code'),
    '#maxlength' => 7,
    '#size' => 7,
    '#element_validate' => array('file_remote_video_vimeo_validate_video_color'),
  );
  $element['vimeo']['protocol_specify'] = array(
    '#title' => t('Specify an http protocol'),
    '#type' => 'checkbox',
    '#default_value' => $settings['protocol_specify'],
    '#description' => t('An explicit protocol may be neccesary for videos embedded in RSS feeds and emails. If no protocol is specified, iframes will be protocol relative.'),
  );
  $element['vimeo']['protocol'] = array(
    '#title' => t('Iframe protocol'),
    '#type' => 'radios',
    '#default_value' => $settings['protocol'],
    '#options' => array(
      'http://' => 'HTTP',
      'https://' => 'HTTPS',
    ),
  );
  $element['vimeo']['autoplay'] = array(
    '#title' => t('Autoplay video on load'),
    '#type' => 'checkbox',
    '#default_value' => $settings['autoplay'],
  );
  $element['vimeo']['loop'] = array(
    '#title' => t('Loop video'),
    '#type' => 'checkbox',
    '#default_value' => $settings['loop'],
  );
  $element['vimeo']['title'] = array(
    '#title' => t('Show the video title'),
    '#type' => 'checkbox',
    '#default_value' => $settings['title'],
  );
  $element['vimeo']['byline'] = array(
    '#title' => t('Show the video byline'),
    '#type' => 'checkbox',
    '#default_value' => $settings['byline'],
  );
  $element['vimeo']['portrait'] = array(
    '#title' => t("Show the user's portrait"),
    '#type' => 'checkbox',
    '#default_value' => $settings['portrait'],
  );
  $element['vimeo']['api'] = array(
    '#title' => t('Enable the <a href="http://developer.vimeo.com/player/js-api">Javascript API</a>'),
    '#type' => 'checkbox',
    '#default_value' => $settings['api'],
    '#description' => t('A required id in the format <code>media-vimeo-{video_id}</code> will be added to each iframe.'),
  );
  return $element;
}

function file_remote_video_field_formatter_settings_form_vimeo_ajax($form, $form_state) {
  $field_name = $form_state['formatter_settings_edit'];
  return $form['fields'][$field_name]['format']['settings_edit_form']['settings']['vimeo'];
}

/**
* Implements hook_field_formatter_settings_summary().
*/
function file_remote_video_field_formatter_settings_summary($field, $instance, $view_mode) {
  $display = $instance['display'][$view_mode];
  $settings = $display['settings'];
  $summary = '';
  if ($display['type'] == 'file_remote_video') {
    if ($settings['youtube_player'] == 'global') {
      $summary .= t('YouTube: global player settings');
    }
    else {
      $summary .= t("YouTube: @width by @height player, @color theme", [
        '@width' => $settings['youtube']['width'],
        '@height' => $settings['youtube']['height'],
        '@color' => $settings['youtube']['color'],
      ]);
    }
    $summary .= '<br>';
    if ($settings['vimeo_player'] == 'global') {
      $summary .= t('Vimeo: global player settings');
    }
    else {
      $summary .= t('Vimeo: @width by @height player', [
        '@width' => $settings['vimeo']['width'],
        '@height' => $settings['vimeo']['height'],
      ]);
    }
    return $summary;
  }
}

/**
 * Implements hook_field_formatter_view().
 */
function file_remote_video_field_formatter_view($entity_type, $entity, $field, $instance, $langcode, $items, $display) {
  $element = array();

  if ($display['type'] == 'file_remote_video') {
    foreach ($items as $delta => $item) {
      if ($item['filemime'] == 'video/youtube') {
        $element[] = file_remote_video_field_formatter_view_youtube($item, $display);
      }
      else if ($item['filemime'] == 'video/vimeo') {
        $element[] = file_remote_video_field_formatter_view_vimeo($item, $display);
      }
    }
  }
  return $element;
}

function file_remote_video_field_formatter_view_youtube($item, $display) {
  $config = config('file_remote_video.settings');
  $config_data = $config->getData();
  $settings = $config_data['youtube'];
  if ($display['settings']['youtube_player'] == 'custom') {
    $settings = $display['settings']['youtube'];
  }
  $file = file_load($item['fid']);

  $element = array(
    '#theme' => 'file_remote_video_youtube',
    '#uri' => $file->uri,
    '#options' => array(),
  );

  $settings['attributes'] = [];
  $settings_keys = array_keys($settings);

  foreach ($settings_keys as $setting) {
    $element['#options'][$setting] = isset($file->override[$setting]) ? $file->override[$setting] : $settings[$setting];
  }

  return $element;
}

function file_remote_video_field_formatter_view_vimeo($item, $display) {
  $config = config('file_remote_video.settings');
  $config_data = $config->getData();
  $settings = $config_data['vimeo'];
  if ($display['settings']['vimeo_player'] == 'custom') {
    $settings = $display['settings']['vimeo'];
  }
  $file = file_load($item['fid']);
  $element = array(
    '#theme' => 'file_remote_video_vimeo',
    '#uri' => $file->uri,
    '#options' => array(),
  );

  $settings['attributes'] = [];
  $settings_keys = array_keys($settings);

  foreach ($settings_keys as $setting) {
    $element['#options'][$setting] = isset($file->override[$setting]) ? $file->override[$setting] : $settings[$setting];
  }

  return $element;
}

/**
 * Validation for width and height.
 */
function file_remote_video_youtube_validate_video_width_and_height($element, &$form_state, $form) {
  // Check if the value is a number with an optional decimal or percentage sign, or "auto".
  if (!empty($element['#value']) && !preg_match('/^(auto|([0-9]*(\.[0-9]+)?%?))$/', $element['#value'])) {
    form_error($element, t("The value entered for @dimension is invalid. Please insert a unitless integer for pixels, a percent, or \"auto\". Note that percent and auto may not function correctly depending on the browser and doctype.", array('@dimension' => $element['#title'])));
  }
}

/**
 * Validation for Js API Origin.
 */
function file_remote_video_youtube_validate_jsapi_domain ($element, &$form_state, $form) {
  // Check if the value is a url with http/s and no trailing directories.
  if (!empty($element['#value']) && !preg_match('/^https?\:\/\/[a-zA-Z0-9\-\.]+\.([a-zA-Z]{2,4}){1,2}$/', $element['#value'])) {
    form_error($element, t('Please insert a valid domain in the format http://www.yourdomain.com'));
  }
}

/**
 * Validation for width and height.
 */
function file_remote_video_vimeo_validate_video_width_and_height($element, &$form_state, $form) {
  // Check if the value is a number with an optional decimal or percentage sign, or "auto".
  if (!empty($element['#value']) && !preg_match('/^(auto|([0-9]*(\.[0-9]+)?%?))$/', $element['#value'])) {
    form_error($element, t("The value entered for @dimension is invalid. Please insert a unitless integer for pixels, a percent, or \"auto\". Note that percent and auto may not function correctly depending on the browser and doctype.", array('@dimension' => $element['#title'])));
  }
}

/**
 * Validation for color.
 */
function file_remote_video_vimeo_validate_video_color($element, &$form_state, $form) {
  // If the value is set, check if it's a hex color with or without '#'.
 if (!empty($element['#value']) && !preg_match('/^#?([0-9a-fA-F]{3}|[0-9a-fA-F]{6})$/', $element['#value'])) {
    form_error($element, t("The color value is invalid. Please use a 3 or 6 character hex color code.", array('@dimension' => $element['#title'])));
  }
}
