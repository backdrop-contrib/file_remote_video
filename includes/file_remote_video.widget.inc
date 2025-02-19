<?php

/**
 * Implements hook_element_info().
 *
 * We have to implement this so we can add a value callback
 * and return an array to the file field like it expects.
 *
 */
function file_remote_video_element_info() {
  $path = backdrop_get_path('module', 'file_remote_video');
  $types['remote_video_widget'] = [
    '#input' => TRUE,
    '#process' => ['file_remote_video_widget_process'],
    '#value_callback' => 'file_remote_video_widget_value',
    '#element_validate' => ['file_remote_video_widget_validate'],
    '#theme' => 'file_remote_video_widget',
    '#theme_wrappers' => ['form_element'],
    '#extended' => TRUE,
    '#attached' => [
      'js' => [$path . '/js/file_remote_video_browser.js'],
      'css' => [$path . '/css/file_remote_video_browser.css'],
    ]
  ];
  return $types;
}


/**
 * Implements hook_field_widget_info().
 */
function file_remote_video_field_widget_info() {
  return [
    'file_remote_video' => [
      'label' => t('Remote Video'),
      'field types' => ['file'],
      'settings' => [
        'progress_indicator' => 'throbber',
      ],
      'behaviors' => [
        'multiple values' => FIELD_BEHAVIOR_CUSTOM,
        'default value' => FIELD_BEHAVIOR_NONE,
      ],
    ],
  ];
}

/**
 * Implements hook_field_widget_form().
 */
function file_remote_video_field_widget_form(&$form, &$form_state, $field, $instance, $langcode, $items, $delta, $element) {
  // required for file fields
  $defaults = [
    'fid' => 0,
    'display' => !empty($field['settings']['display_default']),
    'description' => '',
  ];

  // Load the items for form rebuilds from the field state as they might not be
  // in $form_state['values'] because of validation limitations. Also, they are
  // only passed in as $items when editing existing entities.
  // $field_state['items'] is added by file_remote_video_widget_submit
  $field_state = field_form_get_state($element['#field_parents'], $field['field_name'], $langcode, $form_state);
  $field_state_exists = isset($field_state['items']) ? 'true' : 'false';
  if (isset($field_state['items'])) {
    $items = $field_state['items'];
  }
  $elements = [
    '#type' => 'fieldset',
    '#process' => ['file_remote_video_widget_wrapper_process'],
  ];

  $element_info = element_info('remote_video_widget');
  $element += [
    '#type' => 'remote_video_widget',
    '#field_label' => $instance['label'],
  ];
  if ($field['cardinality'] == 1) {
    // Set the default value.
    $element['#default_value'] = !empty($items) ? $items[0] : $defaults;
    if ($instance['required']) {
      $element['#required'] = TRUE;
    }
    $elements[] = $element;
  }
  else {
    foreach ($items as $key => $item) {
      $elements[$delta] = $element;
      $elements[$delta]['#default_value'] = $item;
      $elements[$delta]['#required'] = ($instance['required'] && $delta == 0);
      $elements[$delta]['#delta'] = $delta;
      $delta++;
    }
    // And then add one more empty row for new uploads except when this is a
    // programmed form as it is not necessary.
    if (($field['cardinality'] == FIELD_CARDINALITY_UNLIMITED || $delta < $field['cardinality']) && empty($form_state['programmed'])) {
      $elements[$delta] = $element;
      $elements[$delta]['#default_value'] = $defaults;
      $elements[$delta]['#required'] = ($instance['required'] && $delta == 0);
      $elements[$delta]['#delta'] = $delta;
    }
  }

  return $elements;
}

function file_remote_video_widget_wrapper_process($element, &$form_state, $form) {
  // Prefix and suffix used for Ajax replacement.
  $element['#prefix'] = '<div id="' . $element['#id'] . '-ajax-wrapper">';
  $element['#suffix'] = '</div>';
  return $element;
}

/**
 * Render API callback: Expands the remote video widget.
 *
 * Expands the file type to include Upload and Remove buttons, as well as
 * support for a default value.
 *
 * This function is assigned as a #process callback in the widget .
 */
function file_remote_video_widget_process($element, &$form_state, $form) {
  $fid = $element['#default_value']['fid'];
  if (isset($element['#value']['fid'])) {
    $fid = $element['#value']['fid'];
  }

  $parents = array_slice($element['#array_parents'], 0, -1);
  $field_element = backdrop_array_get_nested_value($form, $parents);
  $ajax_wrapper = $field_element['#id'] . '-ajax-wrapper';

  $element['#file'] = $fid ? file_load($fid) : FALSE;
  $element['#tree'] = TRUE;
  $element['fid'] = [
    '#type' => 'hidden',
    '#tree' => TRUE,
    '#default_value' => $fid,
  ];
  $element['add'] = [
    '#name' => implode('_', $element['#parents']) . '_add',
    '#type' => 'submit',
    '#value' => t('Add'),
    '#submit' => [],
    '#limit_validation_errors' => [array_slice($element['#parents'], 0, -1)],
    '#ajax' => [
      'callback' => 'file_remote_video_ajax_form_video_browser_dialog',
    ],
  ];
  // This element is triggered automatically when the file browser dialog closes
  $element['refresh'] = [
    '#name' => implode('_', $element['#parents']) . '_refresh',
    '#type' => 'submit',
    '#value' => t('Refresh'),
    '#submit' => ['file_remote_video_widget_submit'],
    '#limit_validation_errors' => [array_slice($element['#parents'], 0, -1)],
    '#ajax' => [
      'callback' => 'file_remote_video_ajax_form_update_file',
      'wrapper' => $ajax_wrapper
    ],
    '#attributes' => ['style' => 'display: none;'],
  ];
  if (!empty($fid)) {
    $element['add']['#value'] = t('Replace Video');
    $element['remove'] = [
      '#name' => implode('_', $element['#parents']) . '_remove',
      '#type' => 'submit',
      '#value' => t('Remove'),
      '#validate' => [],
      '#submit' => ['file_remote_video_widget_submit'],
      '#limit_validation_errors' => [array_slice($element['#parents'], 0, -1)],
      '#ajax' => [
        'callback' => 'file_remote_video_ajax_form_update_file',
        'wrapper' => $ajax_wrapper
      ],
      '#attributes' => ['class' => ['file-remove-button']],
    ];
  }

  return $element;
}

function file_remote_video_widget_validate(&$element, &$form_state) {
  if ($element['#required'] && empty($element['fid']['#value'])) {
    form_set_error($element['#name'], t('@label is required', [
      '@label' => $element['#field_label'],
    ]));
  }
  $value = [
    'fid' => isset($element['fid']['#value']) ? $element['fid']['#value'] : 0,
    'description' => '',
    'display' => 1,
  ];
  form_set_value($element, $value, $form_state);
}

function file_remote_video_widget_submit($form, &$form_state) {
  $parents = $form_state['triggering_element']['#array_parents'];
  $button_key = array_pop($parents);
  $element = backdrop_array_get_nested_value($form, $parents);
  if ($button_key == 'remove') {
    form_set_value($element['fid'], NULL, $form_state);
    backdrop_array_set_nested_value($form_state['input'], $element['fid']['#parents'], NULL);
  }

  // Set the form to rebuild so that $form is correctly updated in response to
  // processing the file removal. Since this function did not change $form_state
  // if the upload button was clicked, a rebuild isn't necessary in that
  // situation and setting $form_state['redirect'] to FALSE would suffice.
  // However, we choose to always rebuild, to keep the form processing workflow
  // consistent between the two buttons.
  $form_state['rebuild'] = TRUE;

  // The following was cribbed directly from file_field_widget_submit
  // This sets $field_state['items'] so we can use them in
  // file_remote_video_widget_form()
  // During the form rebuild, file_remote_video_widget_form() will create field item
  // widget elements using re-indexed deltas, so clear out $form_state['input']
  // to avoid a mismatch between old and new deltas. The rebuilt elements will
  // have #default_value set appropriately for the current state of the field,
  // so nothing is lost in doing this.
  $parents = array_slice($form_state['triggering_element']['#parents'], 0, -2);
  backdrop_array_set_nested_value($form_state['input'], $parents, NULL);

  $button = $form_state['triggering_element'];

  // Go one level up in the form, to the widgets container.
  $element = backdrop_array_get_nested_value($form, array_slice($button['#array_parents'], 0, -1));
  $field_name = $element['#field_name'];
  $langcode = $element['#language'];
  $parents = $element['#field_parents'];

  $submitted_values = backdrop_array_get_nested_value($form_state['values'], array_slice($button['#parents'], 0, -2));
  foreach ($submitted_values as $delta => $submitted_value) {
    if (!$submitted_value['fid']) {
      unset($submitted_values[$delta]);
    }
  }

  // Re-index deltas after removing empty items.
  $submitted_values = array_values($submitted_values);

  // Update form_state values.
  backdrop_array_set_nested_value($form_state['values'], array_slice($button['#parents'], 0, -2), $submitted_values);

  // Update items.
  $field_state = field_form_get_state($parents, $field_name, $langcode, $form_state);
  $field_state['items'] = $submitted_values;

  field_form_set_state($parents, $field_name, $langcode, $form_state, $field_state);
}

function file_remote_video_widget_value(&$element, $input = FALSE, $form_state = NULL) {
  $fid = 0;
  if ($input && isset($input['fid'])) {
    $fid = $input['fid'];
  }

  // Find the current value of this field from the form state.
  $form_state_fid = $form_state['values'];
  foreach ($element['#parents'] as $parent) {
    $form_state_fid = isset($form_state_fid[$parent]) ? $form_state_fid[$parent] : 0;
  }

  if (isset($form_state_fid['fid'])) {
    $fid = $form_state_fid['fid'];
  }

  if ($input == FALSE) {
    $default_fid = isset($element['#default_value']['fid']) ? $element['#default_value']['fid'] : 0;
    $fid = $default_fid;
    $return = isset($element['#default_value']) ? $element['#default_value'] : ['fid' => 0];
  }

  $return['fid'] = $fid;
  $return += [
    'display' => 1,
    'description' => '',
  ];
  return $return;
}

function file_remote_video_ajax_form_update_file($form, &$form_state) {
  // Go up two levels to get the widget wrapping element
  $parents = array_slice($form_state['triggering_element']['#parents'], 0, -2);
  $element = backdrop_array_get_nested_value($form, $parents);
  return $element;
}

function file_remote_video_ajax_form_video_browser_dialog($form, &$form_state) {
  $parents = $form_state['triggering_element']['#array_parents'];
  array_pop($parents);
  $element = backdrop_array_get_nested_value($form, $parents);

  $add_form = backdrop_get_form('file_remote_video_form_add_file', $element['fid']['#name']);

  $title = t('Select from Image Library');
  $options = array(
    'dialogClass' => 'file-remote-video-browser-container',
    'width' => '90%',
    'modal' => TRUE,
  );
  $view = views_embed_view('file_remote_video_library', 'block');
  $html = '';
  $html .= theme('status_messages');
  $html .= '<div class="file-remote-video-browser">';
  $html .= '<div class="file-remote-videos-tabs-wrapper">';
  $html .= '<ul class="file-remote-video-tabs">';
  $html .= '<li class="tab"><a href="#" data-op="add" data-selector="#file-remote-video-form-wrapper">' . t('Add Video') . '</a>';
  $html .= '<li class="tab"><a href="#" data-op="insert" data-selector=".file-remote-video-browser-view">' . t('Browse video library') . '</a>';
  $html .= '</ul>';
  $html .= '</div>';
  $html .= '<div class="file-remote-video-browser-view">';
  $html .= $view;
  $html .= '</div>';
  $html .= backdrop_render($add_form);
  $html .= '</div>';

  $settings = [
    'fileRemoteVideoBrowser' => [
      'selectedFid' => NULL,
      'fidElement' => $element['fid']['#name'],
      'refreshButton' => $element['refresh']['#name'],
    ],
  ];

  $commands = array();
  $commands[] = ajax_command_settings($settings, TRUE);
  $commands[] = ajax_command_open_dialog('#file-remote-video-browser-modal', $title, $html, $options);
  return array('#type' => 'ajax', '#commands' => $commands);
}

function file_remote_video_form_add_file($form, &$form_state, $field_name) {
  // FID is set via JavaScript on the client-side. As such, this is a "hidden"
  // element, rather than "value". And we use #default_value, not #value, to
  // allow client-side changes.
  $form['#prefix'] = '<div id="file-remote-video-form-wrapper">';
  $form['#suffix'] = '</div>';
  $form['messages'] = [
    '#type' => 'markup',
    '#prefix' => '<div id="file-remote-video-add-messages">',
    '#markup' => '',
    '#suffix' => '</div>',
  ];
  $form['frv_op'] = [
    '#type' => 'hidden',
    '#default_value' => 'add',
  ];
  $form['add'] = [
    '#type' => 'fieldset',
    '#title' => t('Add new video'),
    '#prefix' => '<div id="file-remote-video-add">',
    '#suffix' => '</div>',
  ];
  $form['add']['fid'] = [
    '#type' => 'hidden',
    '#default_value' => '',
  ];
  $form['add']['field_name'] = [
    '#type' => 'hidden',
    '#value' => $field_name,
  ];
  $form['add']['url'] = [
    '#type' => 'textfield',
    '#title' => t('Url to Video'),
    '#element_validate' => ['file_remote_video_file_url_validate'],
  ];
  $form['add']['actions']['#type'] = 'actions';
  $form['add']['actions']['submit'] = [
    '#type' => 'submit',
    '#value' => t('Insert Video'),
    '#submit' => [],
    '#ajax' => [
      'callback' => 'file_remote_video_ajax_browser_add_submit',
      'wrapper' => 'file-remote-video-form-wrapper',
      'progress' => [
        'type' => 'throbber',
        'message' => NULL,
      ],
    ],
  ];
  $form['add']['actions']['cancel'] = [
    '#type' => 'link',
    '#title' => t('Cancel'),
    '#href' => '',
    '#attributes' => [
      // Adding button classes puts the link in the button area of the dialog,
      // while the special "dialog-cancel" class closes the dialog.
      'class' => [
        'button',
        'button-secondary',
        'form-submit',
        'dialog-cancel',
      ],
    ],
  ];

  return $form;
}

function file_remote_video_file_url_validate($element, &$form_state) {
  $raw_url = $form_state['values']['url'];
  $validate = TRUE;
  if (empty($raw_url)) {
    $validate = FALSE;
  }
  if (isset($form_state['values']['frv_op']) && $form_state['values']['frv_op'] == 'add') {
    $validate = TRUE;
  }
  if ($validate) {
    $url = parse_url($form_state['values']['url']);
    if ($url == FALSE) {
      form_error($element, t('Please enter a valid url'));
      return;
    }
    $handlers = file_remote_video_get_remote_video_handlers();
    $valid_domains = [];
    $handler_class = NULL;
    foreach ($handlers as $handler) {
      $valid_domains = array_merge($valid_domains, $handler['class']::VALID_DOMAINS);
      if (isset($url['host']) && in_array($url['host'], $handler['class']::VALID_DOMAINS)) {
        $handler_class = $handler['class'];
      }
    }
    if (!isset($url['host']) || $handler_class == NULL) {
      form_error($element, t('Please enter a url from one of the following domains: @domains', [
        '@domains' => implode(', ', $valid_domains)
      ]));
    }
    else {
      $handler = new $handler_class($raw_url);
      if ($handler->claim($raw_url)) {
        try {
          $file_uri = $handler->parse($raw_url);
          $form_state['file_uri'] = $file_uri;
          $form_state['handler'] = $handler;
        }
        catch(Throwable $e) {
          form_error($element, t('Error parsing video url: @message'), [
            '@message' => $e->getMessage(),
          ]);
        }
      }
      else {
        form_error($element, t('Invalid video url. Please enter a valid video url.'));
      }
    }
  }
}

function file_remote_video_ajax_browser_add_submit(&$form, &$form_state) {
  $parents = $form_state['triggering_element']['#array_parents'];
  $element = backdrop_array_get_nested_value($form, $parents);
  $errors = form_get_errors();
  $commands = [];
  if ($errors) {
    $form['messages']['#markup'] = theme('status_messages');
    return $form;
  }
  else {
    if ($form_state['values']['frv_op'] == 'add') {
      $file = file_remote_video_ajax_browser_save_file($form_state['file_uri'], $form_state['handler']);
    }
    else if ($form_state['values']['frv_op'] == 'insert') {
      $file = file_load($form_state['values']['fid']);
    }
    if ($file && $file->fid) {
      $settings = [
        'fileRemoteVideoBrowser' => [
          'selectedFid' => (int) $file->fid,
          'currentFidElement' => $form_state['values']['field_name'],
        ],
      ];
      $commands[] = ajax_command_settings($settings, TRUE);
      $commands[] = ajax_command_close_dialog('#file-remote-video-browser-modal');
    }
    return [
      '#type' => 'ajax',
      '#commands' => $commands,
    ];
  }
}

function file_remote_video_ajax_browser_save_file($file_uri, $handler) {
  $file = file_remote_video_load_or_create_video_by_uri($file_uri);
  if (!$file->fid) {
    if (method_exists($handler, 'getFileInfo')) {
      $info = $handler->getFileInfo();
      $file->filename = truncate_utf8($info['title'], 255);
    }
    $file->save();
  }
  return $file;
}

function file_remote_video_form_ajax_browser_submit($form, &$form_state) {
  $commands = [];
  if (!empty($form_state['values']['fid'])) {
    $settings = [
      'file' => [
        'browser' => [
          'selectedFid' => (int) $form_state['values']['fid'],
        ],
      ],
    ];
    $commands[] = ajax_command_settings($settings, TRUE);
  }
  $commands[] = ajax_command_close_dialog('#file-remote-video-browser-modal');
  return ['#type' => 'ajax', '#commands' => $commands];
}
