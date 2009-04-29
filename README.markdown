README
=======================

services_oop allows declarative definition of resource-based services through class declarations.

Example
-----------------------

This is a incomplete example of how a resource could be implemented.

    <?php

    /**
     * Class that defines the event resource
     *
     * @Relationship(name='occasions', controller='retrieveOccasions')
     */
    class EventResource {
      /**
       * Creates a event
       *
       * @param object $event ["data"]
       * @return object
       *
       * @Access(callback='_event_resource_access', args={'create'}, appendArgs=true)
       */
      public static function create($event) {
        // Do your thing here
      }

      /**
       * Retrieves a event
       *
       * @param int $nid ["path","0"]
       *  The nid of the event to get
       * @return object
       *
       * @Access(callback='_event_resource_access', args={'view'}, appendArgs=true)
       * @Model(class='ResourceFeedModel', implementation='EventResourceFeedModel')
       */
      public static function retrieve($nid) {
        // Do your thing here
      }

      /**
       * Updates a event
       *
       * @param int $nid ["path","0"]
       *  The nid of the event to update
       * @param object $event ["data"]
       *  The event object
       * @return object
       *
       * @Access(callback='_event_resource_access', args={'update'}, appendArgs=true)
       */
      public static function update($nid, $event) {
        // Do your thing here
      }

      /**
       * Deletes a event
       *
       * @param int $nid ["path","0"]
       *  The nid of the event to get
       * @return bool
       *
       * @Access(callback='_event_resource_access', args={'delete'}, appendArgs=true)
       */
      public static function delete($nid) {
        // Do your thing here
      }

      /**
       * Retrieves a listing of events
       *
       * @param int $page ["param","page"]
       * @param string $fields ["param","fields"]
       * @param array $params ["param"]
       * @return array
       *
       * @Access(callback='user_access', args={'access content'}, appendArgs=false)
       * @Model(class='ResourceFeedModel', implementation='EventResourceFeedModel')
       * @Model(class='ResourceTimeFeedModel', implementation='EventResourceFeedModel')
       */
      public static function index($page=0, $fields=array(), $params=array()) {
        // Do your thing here
      }

      /**
       * Controller method for getting occasions for a event
       *
       * @param int $nid ["path","0"]
       *  The nid of the event to get occasions for
       * @param int $page ["param","page"]
       * @return array
       *
       * @Model(class='ResourceFeedModel', implementation='EventOccasionsFeedModel')
       * @Model(class='ResourceTimeFeedModel', implementation='EventOccasionsFeedModel')
       */
      public static function retrieveOccasions($nid, $page=0) {
        // Do your thing here
      }
    }